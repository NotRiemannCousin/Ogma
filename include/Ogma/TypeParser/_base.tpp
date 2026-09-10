#pragma once

//! @file Ogma/TypeParser/_base.tpp
//! @brief Definitions of the TypeParser<T> primary template: the default
//!        struct-walking engine (the per-token loop and its tables).
//!
//! The reflection plumbing it calls lives in @c _internal.tpp; the per-type
//! specialisations are pulled in at the bottom. Included by
//! @c <Ogma/TypeParser/_base.hpp> (directly or via @c _internal.tpp), so the
//! primary fallback is always available.

#include <algorithm>
#include <flat_set>
#include <format>
#include <utility>

#include <Ogma/TypeParser/_internal.tpp>

namespace Ogma {

    namespace details_ {

        constexpr std::string ToScreamingSnakeCase(std::string_view memberName) {
            using namespace Utils;

            std::string result{ "<" };

            if (!memberName.empty())
                result += ToUpper(memberName.front());

            for (auto [prev, curr] : memberName | std::views::pairwise) {
                if (IsLower(prev) && IsUpper(curr)) result += '_';
                result += ToUpper(curr);
            }

            result += '>';
            return result;
        }


        template<class T, Arg ArgDefinition, auto... Annotations>
        consteval ValueSpec DefaultSpec(std::meta::info member) {
            using std::string_literals::operator ""s;
            namespace vs = std::views;

            constexpr auto joinFlags{ vs::filter(Utils::notEmpty) | vs::transform(Utils::asFlag) | Utils::joinByComma };

            return {
                .usageFlag = Utils::asFlag(ArgDefinition.longName),
                .flagList  = TypeParser<T, ArgDefinition>::template ExpectedFlags<Annotations...>() | joinFlags,
                .metavars  = ToScreamingSnakeCase(std::meta::identifier_of(member)),
            };
        }
    }



    template<class T, Arg ArgDefinition>
    template<auto...>
    Result<T> TypeParser<T, ArgDefinition>::Parse(T, std::string_view, ArgSpan args,
                                                  std::string_view scopePath) {
        static constexpr auto named{ details_::NamedArgChoices<T>() };
        static constexpr auto positional{ details_::PositionalArgChoices<T>() };

        const std::string_view selfPath{ scopePath.empty() ? ScopePathOf<T>() : scopePath };

        T result{};
        std::flat_set<std::string_view> filledArgs{};

        while (!args.empty()) {
            if (args.front() == closingToken) {
                args = args.subspan(1);
                break;
            }

            const std::string_view token{ args.front() };
            const bool isOption{ Utils::IsFlag(token) };

            std::expected<details_::ArgumentInfo<T>, std::string> matched{ std::unexpect };
            if (isOption) {
                const std::string_view flagName{ token.substr(2) };

                if (flagName == "help")
                    throw HelpRequested{ RenderHelpPage<T>(selfPath) };
                if constexpr (HasAppInfo<T>()) {
                    if (flagName == "version")
                        throw HelpRequested{ RenderVersion<T>() };
                }

                const auto it{ std::ranges::lower_bound(
                    named, flagName, std::less<>{}, &details_::NamedEntry<T>::first) };
                if (it != named.end() && flagName == it->first)
                    matched = it->second;
                else
                    matched = std::unexpected{
                        std::format("unknown option: '{}'", token) };
            } else {
                const auto it{ std::ranges::find_if(
                    positional,
                    [&](const details_::ArgumentInfo<T>& info) {
                        return !filledArgs.contains(info.identifier);
                    }) };
                if (it != positional.end())
                    matched = *it;
                else
                    matched = std::unexpected{
                        std::format("unexpected argument: '{}'", token) };
            }
            if (!matched)
                return std::unexpected{ std::move(matched).error() };

            if (auto rest{ std::invoke(matched->setter, result, token, args, selfPath) }) {
                args = *rest;
            } else {
                std::string error{ std::move(rest).error() };
                if (isOption)
                    return std::unexpected{ std::format("{}: {}", token, std::move(error)) };
                return std::unexpected{ std::format("{}: {}", matched->identifier, std::move(error)) };
            }

            filledArgs.insert(matched->identifier);
        }

        std::string missing{ details_::MissingRequired<T>(filledArgs) };
        if (!missing.empty())
            return std::unexpected{ std::move(missing) };

        return std::make_pair(std::move(result), args);
    }

    template<class T, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<T, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 1> list{ ArgDefinition.longName };
        return list;
    }

    template<class T, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<T, ArgDefinition>::Spec(const std::meta::info member) {
        return details_::DefaultSpec<T, ArgDefinition, Annotations...>(member);
    }

}

#include <Ogma/TypeParser/Help.tpp>
#include <Ogma/TypeParser/_base/_pch.hpp>
