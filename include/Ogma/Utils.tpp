#pragma once

//! @file Ogma/Utils.tpp
//! @brief Definitions of the reflection helpers declared by Utils.hpp.

#include <algorithm>
#include <ranges>
#include <stdexcept>

namespace Ogma { template<auto...> struct Fwd; }

namespace Ogma::Utils {
    // I hate <cctype> with all of my forces

    constexpr bool IsLower(const char c) { return c >= 'a' && c <= 'z'; }
    constexpr bool IsUpper(const char c) { return c >= 'A' && c <= 'Z'; }
    constexpr char ToLower(const char c) { return static_cast<char>(IsUpper(c) ? c - 'A' + 'a' : c); }
    constexpr char ToUpper(const char c) { return static_cast<char>(IsLower(c) ? c - 'a' + 'A' : c); }


#pragma region Reflection

    template<std::meta::info ...Ts>
    consteval bool IsOneOf(const std::meta::info ref) {
        if (!std::meta::is_nonstatic_data_member(ref))
            return false;

        const std::size_t count{
            (std::meta::annotations_of_with_type(ref, Ts).size() + ...) };
        if (count > 1)
            throw std::runtime_error{
                "IsOneOf: multiple annotations of the same type" };

        return count == 1;
    }

    template<std::meta::info ...Ts>
    consteval std::meta::info GetOf(const std::meta::info ref) {
        if (!std::meta::is_nonstatic_data_member(ref) || !IsOneOf<Ts...>(ref))
            throw std::runtime_error{ "GetOf: invalid member" };

        template for (constexpr std::meta::info target : { Ts... })
            for (std::meta::info annotation :
                 std::meta::annotations_of_with_type(ref, target))
                return annotation;

        throw std::runtime_error{ "GetOf: invalid state" };
    }

    template<std::meta::info ...Ts>
    consteval std::vector<std::pair<std::meta::info, std::meta::info>> MatchedMembers(std::meta::info type) {
        std::vector<std::pair<std::meta::info, std::meta::info>> matched;

        for (std::meta::info ref :
             std::meta::nonstatic_data_members_of(type, std::meta::access_context::current()))
            if (IsOneOf<Ts...>(ref))
                matched.emplace_back(ref, GetOf<Ts...>(ref));

        return matched;
    }

    consteval std::vector<std::meta::info> AnnotationValues(const std::meta::info member) {
        return { std::from_range, std::meta::annotations_of(member) | std::views::transform(&std::meta::constant_of) };
    }

#pragma endregion



#pragma region Flags


    constexpr bool IsFlag(const std::string_view token) {
        return token.starts_with(controlToken) && token.size() > 2;
    }

    constexpr bool IsValidFlagName(const std::string_view name) {
        if (name.empty() || name.front() == '-' || name.back() == '-')
            return false;
        constexpr auto isAllowed{ [](const char c) {
            return c == '-' || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9');
        } };

        return std::ranges::all_of(name, isAllowed);
    }

#pragma endregion



#pragma region Variety

    template<class T>
    consteval bool IsSpecializationOf(const std::meta::info templ) {
        constexpr std::meta::info info{ ^^T };
        return std::meta::has_template_arguments(info) && std::meta::template_of(info) == templ;

    }

    template<class Tag, auto... Annotations>
    consteval std::optional<Tag> GetAnnotation() {
        template for (constexpr auto annotation : { Annotations... })
            if constexpr (std::same_as<std::remove_cvref_t<decltype(annotation)>, Tag>)
                return annotation;
        return std::nullopt;
    }

    template<auto Value>
    consteval std::meta::info ReflectValue() {
        constexpr auto v{ Value };
        return std::meta::constant_of(^^v);
    }

    template<auto... Annotations>
    consteval std::vector<std::meta::info> GetForwardedAnnots() {
        std::vector<std::meta::info> annots{ ReflectValue<Annotations>()... };
        namespace vs = std::views;
        namespace rg = std::ranges;
        namespace mt = std::meta;

        constexpr auto getForwarded{ [](const mt::info annot) -> std::vector<mt::info> {
            const auto type{ mt::type_of(annot) };
            if (mt::has_template_arguments(type) && mt::template_of(type) == ^^Fwd)
                return mt::template_arguments_of(type);
            return {};
        } };

        return { std::from_range, annots | vs::transform(getForwarded) | vs::join };
    }

    template<class T>
    constexpr std::expected<T, std::string> GetNumber(std::string_view value, auto... args) {
        T number{};
        const auto [ptr, ec]{ std::from_chars(value.data(), value.data() + value.size(), number, args...) };
        if (ec != std::errc{} || ptr != value.data() + value.size())
            return std::unexpected{ std::format("invalid number: '{}'", value) };
        return number;
    }

    template<auto F>
    constexpr auto
    ConsumeOneAndForward(ArgSpan args) -> Result<std::remove_cvref_t<decltype(*std::invoke(F, args[0]))>> {
        const auto joinRemainingArgs{ [args](const auto&& res) {
            return std::pair{ std::move(res), args.subspan(1) };
        } };

        if (args.empty()) return std::unexpected{ std::format("unexpected end of args") };

        return std::invoke(F, args[0]).transform(joinRemainingArgs);
    }

    template<class T>
    constexpr auto ConsumeOneAndForward(ArgSpan args) {
        return ConsumeOneAndForward<T{}>(args);
    }

#pragma endregion


}
