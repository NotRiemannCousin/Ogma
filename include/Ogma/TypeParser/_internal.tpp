#pragma once

//! @file Ogma/TypeParser/_internal.tpp
//! @brief Implementation details of the default TypeParser engine.
//!
//! Setters, consteval table builders and the missing-required reporter.
//! Not public API.

#include <cstddef>
#include <flat_map>
#include <flat_set>
#include <expected>
#include <format>
#include <functional>
#include <meta>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Ogma::details_ {
    template<class T>
    using MemberSetter = std::expected<ArgSpan, std::string>(T&, std::string_view, ArgSpan, std::string_view);


    template<class T>
    struct ArgumentInfo {
        const char* identifier{};
        MemberSetter<T>* setter{};
    };

    template<class T>
    using NamedEntry = std::pair<const char*, ArgumentInfo<T>>;

    template<class T>
    using ParseFnMap = std::flat_map<std::string_view, ArgumentInfo<T>>;
    template<class T>
    using RawParseFnMap = std::span<const NamedEntry<T>>;
    template<class T>
    using ParseFnVector = std::vector<ArgumentInfo<T>>;
    template<class T>
    using RawParseFnVector = std::span<const ArgumentInfo<T>>;

    template<class T, auto Mem, Arg arg>
    std::expected<ArgSpan, std::string> ParseMember(T& parent, const std::string_view token, ArgSpan args,
                                                    const std::string_view scope) {
        using FieldT = TypeOf<Mem>;
        using Parser = TypeParser<FieldT, arg>;

        static constexpr auto annotations{ std::define_static_array(Utils::AnnotationValues(Mem)) };
        static constexpr auto parseFn{ std::meta::substitute(^^Parser::Parse, annotations) };

        const bool isFlag{ Utils::IsFlag(token) };
        const std::string_view flag{ isFlag ? token : std::string_view{} };
        const ArgSpan rest{ isFlag ? args.subspan(1) : args };

        const std::string memberScope{ std::vector{ scope, flag } | Utils::concatBySpace };

        auto& value{ parent.[: Mem :] };
        auto parsed{ std::invoke([: parseFn :], std::move(value), flag, rest, memberScope) };

        if (!parsed)
            return std::unexpected{ std::move(parsed).error() };

        value = std::move(parsed->first);
        return parsed->second;
    }

    template<class T, auto Mem, VariantArg arg>
    std::expected<ArgSpan, std::string> ParseMember(T& parent, const std::string_view token, ArgSpan args,
                                                    const std::string_view scopePath) {
        using VariantT = TypeOf<Mem>;
        constexpr auto [...I]{ std::make_index_sequence<arg.caseCount>{} };

        template for (constexpr auto branchIndex : { I... }) {
            if (token == arg.cases[branchIndex].longName) {
                using AltT = std::variant_alternative_t<branchIndex, VariantT>;
                const std::string branchPath{
                    std::format("{} {}", scopePath, arg.cases[branchIndex].longName) };
                const ArgSpan rest{ args.subspan(1) };

                auto parsed{ [&] {
                    return TypeParser<AltT, internalArg>::Parse({}, "", rest, branchPath);
                }() };
                if (!parsed)
                    return std::unexpected{
                    std::format("{}: {}", arg.cases[branchIndex].longName,
                                std::move(parsed).error()) };

                parent.[: Mem :] = std::move(parsed->first);
                return parsed->second;
            }
        }

        return std::unexpected{ std::format("unknown subcommand: '{}'", token) };
    }


    template<class T>
    consteval RawParseFnMap<T> NamedArgChoices() {
        namespace mt = std::meta;

        static constexpr auto members{
            std::define_static_array(Utils::MatchedMembers<^^Arg>(^^T)) };

        ParseFnMap<T> availableFlags{};

        auto tryEmplace{ [&](std::string_view flag, const char* identifier, MemberSetter<T>* setter) {
            if (flag == "help")
                throw std::runtime_error{ std::string{ "flag 'help' is reserved (member '" } + identifier + "' of " + mt::identifier_of(^^T) + ")" };
            if (flag == "version" && HasAppInfo<T>())
                throw std::runtime_error{ std::string{ "flag 'version' is reserved when the app declares AppInfo (member '" } + identifier + "' of " + mt::identifier_of(^^T) + ")" };

            if (!Utils::IsValidFlagName(flag))
                throw std::runtime_error{ std::string{ "invalid flag name: '" } + flag + "'" };
            if (!availableFlags.try_emplace(flag, identifier, setter).second)
                throw std::runtime_error{
                    std::string{ "duplicated flag \"" } + flag + "\" for class "
                    + mt::identifier_of(^^T) };
        } };

        template for (constexpr auto entry : members) {
            constexpr auto member{ entry.first };
            constexpr Arg arg{ mt::extract<Arg>(entry.second) };

            // "" is the positional marker and skipped below, so an empty longName
            // would otherwise bypass validation entirely.
            if (!Utils::IsValidFlagName(arg.longName))
                throw std::runtime_error{
                    std::string{ "invalid flag name: '" } + std::string{ arg.longName } + "'" };

            using FieldT = TypeOf<member>;
            constexpr auto annotations{
                std::define_static_array(Utils::AnnotationValues(member)) };
            constexpr auto expectedFlagsFn{
                mt::substitute(^^TypeParser<FieldT, arg>::ExpectedFlags, annotations) };
            constexpr MemberSetter<T>* setterFn{ &ParseMember<T, member, arg> };
            constexpr const char* identifier{ std::define_static_string(mt::identifier_of(member)) };

            for (const auto flag : std::invoke([: expectedFlagsFn :]))
                if (flag != "")
                    tryEmplace(flag, identifier, setterFn);
        }

        constexpr auto canonizeKeyStrings{ [](const auto& entry) {
            return std::pair{ std::define_static_string(entry.first), entry.second };
        } };

        std::vector<NamedEntry<T>> promoted{
            std::from_range, availableFlags | std::views::transform(canonizeKeyStrings) };

        return std::define_static_array(promoted);
    }

    template<class T>
    consteval RawParseFnVector<T> PositionalArgChoices() {
        namespace mt = std::meta;
        namespace rg = std::ranges;
        namespace vs = std::views;

        static constexpr auto members{
            std::define_static_array(Utils::MatchedMembers<^^Arg, ^^VariantArg>(^^T)) };

        ParseFnVector<T> positionalMembers{};
        bool commandSeen{};

        template for (constexpr auto entry : members) {
            constexpr auto member{ entry.first };
            constexpr auto annotation{ entry.second };

            using MemberType = TypeOf<member>;
            constexpr const char* identifier{ std::define_static_string(mt::identifier_of(member)) };

            if constexpr (mt::type_of(annotation) == ^^const VariantArg) {
                if (commandSeen)
                    throw std::runtime_error{
                        std::string{ "multiple VariantArg members in " }
                        + mt::identifier_of(^^T)
                        + ": a struct declares at most one subcommand" };

                constexpr VariantArg variant{ mt::extract<VariantArg>(annotation) };

                const auto cases{ vs::counted(variant.cases, variant.caseCount) };

                static constexpr auto isValidName{ &Utils::IsValidFlagName };
                static constexpr auto getName{ &VariantArg::VariantBranch::longName };
                static constexpr auto duplicated{ [](const auto& pair) {
                    const auto& [a, b]{ pair };
                    return &a != &b && std::string_view{ a.longName } == std::string_view{ b.longName };
                } };

                if (const auto it{ rg::find_if_not(cases, isValidName, getName) }; it != cases.end())
                    throw std::runtime_error{ "invalid subcommand name: '" + std::string{ it->longName } + "'" };

                for (auto name : vs::cartesian_product(cases, cases)
                        | vs::filter(duplicated)
                        | vs::keys
                        | vs::transform(getName))
                    throw std::runtime_error{ "duplicated subcommand name: '" + std::string{ name } + "'" };


                positionalMembers.emplace_back(identifier, &ParseMember<T, member, variant>);
                commandSeen = true;
            } else {
                if (commandSeen)
                    throw std::runtime_error{
                        std::string{ "member '" } + mt::identifier_of(member)
                        + "' of " + mt::identifier_of(^^T)
                        + " is declared after the VariantArg member: the subcommand"
                          " must be the last annotated member of a struct" };

                if constexpr (IsPositional<MemberType>(Utils::AnnotationValues(member))) {
                    constexpr Arg arg{ mt::extract<Arg>(annotation) };
                    positionalMembers.emplace_back(identifier, &ParseMember<T, member, arg>);
                }
            }
        }

        return std::define_static_array(positionalMembers);
    }

    template<class T>
    std::string MissingRequired(const std::flat_set<std::string_view>& filled) {
        namespace vs = std::views;
        namespace rg = std::ranges;
        namespace mt = std::meta;

        static constexpr auto members{
            std::define_static_array(Utils::MatchedMembers<^^Arg, ^^VariantArg>(^^T)) };

        std::string missing;

        template for (constexpr auto entry : members) {
            constexpr auto member{ entry.first };
            constexpr auto annotation{ entry.second };

            constexpr std::string_view identifier{
                std::define_static_string(mt::identifier_of(member)) };
            if (filled.contains(identifier))
                continue;

            if constexpr (Utils::IsOneOf<^^VariantArg>(member)) {
                constexpr VariantArg variant{ mt::extract<VariantArg>(annotation) };
                const auto options{
                    vs::counted(variant.cases, variant.caseCount)
                    | vs::transform(&VariantArg::VariantBranch::longName)
                    | vs::transform([](const auto& name) {
                          return std::string_view{ name };
                      })
                    | Utils::joinByBar
                    | rg::to<std::string>()
                };
                missing += std::format(
                    "  missing required subcommand (one of: {})\n", options);
            } else if constexpr (Utils::IsOneOf<^^Arg>(member)) {
                using FieldT = TypeOf<member>;
                if constexpr (!Utils::IsSpecializationOf<FieldT>(^^std::optional)) {
                    constexpr Arg arg{ mt::extract<Arg>(annotation) };
                    missing += std::format(
                        "  missing required argument: --{}\n",
                        arg.longName);
                }
            } else {
                // Unreachable: MatchedMembers only yields annotated members.
                throw std::runtime_error{
                    "MissingRequired: invalid member: "
                    + std::string{ mt::identifier_of(member) } };
            }
        }

        return missing;
    }

}