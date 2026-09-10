#pragma once

#include <array>
#include <cctype>
#include <flat_map>
#include <format>

namespace Ogma {

    namespace details_ {
        struct CaseInsensitiveLess {
            using is_transparent = void;

            constexpr bool operator()(std::string_view lhs, std::string_view rhs) const {
                return std::ranges::lexicographical_compare(
                    lhs | std::views::transform(&Utils::ToLower),
                    rhs | std::views::transform(&Utils::ToLower)
                );
            }
        };

        template<class T, bool IsCaseInsensitive>
        using FlatMap = std::flat_map<
            std::string_view, T,
            std::conditional_t<IsCaseInsensitive, CaseInsensitiveLess, std::less<>>
        >;

        template<class E, bool IsCaseInsensitive>
            requires std::is_enum_v<E>
        consteval std::span<const std::pair<const char*, E>> EnumChoices() {
            FlatMap<E, IsCaseInsensitive> availableValues{};

            const auto tryEmplace{ [&](std::string_view key, E enumVal) {
                if (!availableValues.emplace(key, enumVal).second)
                    throw std::runtime_error{ std::string{ "duplicated key \"" } + key + "\" for enum " + std::meta::identifier_of(^^E) };
            } };

            for (auto enumMember : std::meta::enumerators_of(^^E)) {
                const E memberValue{ std::meta::extract<E>(enumMember) };
                tryEmplace(std::meta::identifier_of(enumMember), memberValue);

                for (const auto alias : std::meta::annotations_of_with_type(enumMember, ^^Alias))
                    tryEmplace(std::meta::extract<Alias>(alias).name, memberValue);
            }

            constexpr auto canonizeKeyStrings{ [](const auto& pair) {
                return std::pair{ std::define_static_string(pair.first), pair.second };
            } };

            return std::define_static_array(availableValues | std::views::transform(canonizeKeyStrings));
        }
    }

    template<class T, Arg ArgDefinition>
        requires std::is_enum_v<T>
    template<auto... Annotations>
    Result<T> TypeParser<T, ArgDefinition>::Parse(T, std::string_view, ArgSpan args, std::string_view) {
        constexpr bool isCaseInsensitive{
            Utils::GetAnnotation<CaseInsensitive, Annotations...>()              // CaseInsensitive on var
            || std::meta::annotations_of_with_type(^^T, ^^CaseInsensitive).size()   // CaseInsensitive on type
        };
        constexpr auto joinKeys{ std::views::keys | Utils::joinByComma };
        constexpr auto enumChoices{ details_::EnumChoices<T, isCaseInsensitive>() };

        const details_::FlatMap<T, isCaseInsensitive> availableChoices{ std::from_range, enumChoices };

        if (args.empty())
            return std::unexpected{ std::format("expected one of: {:s}", availableChoices | joinKeys) };


        if (const auto matched{ availableChoices.find(args[0]) }; matched != availableChoices.end())
            return std::pair{ matched->second, args.subspan(1) };

        return std::unexpected{ std::format( "invalid value '{}'; expected one of: {:s}",
                                            args[0], availableChoices | joinKeys) };
    }

    template<class T, Arg ArgDefinition>
        requires std::is_enum_v<T>
    template<auto...>
    constexpr ArgSpan TypeParser<T, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }


    template<class T, Arg ArgDefinition>
        requires std::is_enum_v<T>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<T, ArgDefinition>::Spec(std::meta::info member) {
        using namespace std::string_view_literals;

        constexpr bool isCaseInsensitive{
            Utils::GetAnnotation<CaseInsensitive, Annotations...>()              // CaseInsensitive on var
            || std::meta::annotations_of_with_type(^^T, ^^CaseInsensitive).size()   // CaseInsensitive on type
        };
        constexpr auto toStrView{ [](const auto str){ return std::string_view{ str }; } };

        constexpr auto join{ std::views::transform(toStrView) | std::views::join_with("|"sv) };
        constexpr auto enumNames{ details_::EnumChoices<T, isCaseInsensitive>() | std::views::keys };

        ValueSpec spec{ details_::DefaultSpec<T, ArgDefinition, Annotations...>(member) };

        spec.metavars = enumNames | join | std::ranges::to<std::string>();
        if (isCaseInsensitive)
            spec.modifiers = "[case insensitive]";

        return spec;
    }

}
