#pragma once

#include <array>

namespace Ogma {

    template<class T, Arg ArgDefinition>
    template<auto... Annotations>
    Result<std::optional<T>> TypeParser<std::optional<T>, ArgDefinition>::Parse(
        std::optional<T>, std::string_view flag, ArgSpan args, const std::string_view scopePath) {
        constexpr auto parseFn{ std::meta::substitute(
                    ^^TypeParser<T, ArgDefinition>::Parse,
                    Utils::GetForwardedAnnots<Annotations...>()) };

        auto result{ std::invoke([: parseFn :], T{}, flag, args, scopePath) };
        if (!result) return std::unexpected{ std::move(result).error() };
        return std::make_pair(std::optional<T>{ std::move(result->first) }, result->second);
    }

    template<class T, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::optional<T>, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 1> list{ ArgDefinition.longName };
        return list;
    }


        template<class T, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::optional<T>, ArgDefinition>::Spec(std::meta::info member) {
        constexpr auto innerFn{ std::meta::substitute(
            ^^TypeParser<T, ArgDefinition>::Spec,
            Utils::GetForwardedAnnots<Annotations...>()) };

        ValueSpec spec{ std::invoke([: innerFn :], member) };

        spec.usageFlag = Utils::asFlag(ArgDefinition.longName);
        spec.flagList  = spec.usageFlag;


#ifdef __cpp_lib_constexpr_format
        constexpr bool isInitialized{ std::meta::has_default_member_initializer(member) };
        constexpr bool isConstexpr{ std::integral_constant<ParentT, ParentT{}>() };

        using ParentT = typename [: std::meta::parent_of(member) :];

        if constexpr  (isInitialized && isConstexpr && std::formattable<T, char>) {
            ParentT declaredDefault{};

            if (declaredDefault.has_value())
                spec.modifiers = std::vector<std::string>{
                        spec.modifiers,
                        std::format("[default: {}]", declaredDefault.[: member :])
                    } | Utils::concatBySpace;
        }
#endif

        return spec;
    }
}
