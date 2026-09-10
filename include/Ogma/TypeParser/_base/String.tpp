#pragma once

#include <array>

namespace Ogma {

    template<Arg ArgDefinition>
    template<auto...>
    Result<std::string> TypeParser<std::string, ArgDefinition>::Parse(
        std::string, const std::string_view flag, ArgSpan args, std::string_view) {

        if (flag.empty() && !args.empty() && args[0] == controlToken) args = args.subspan(1);
        if (args.empty()) return std::unexpected{ "expected a string" };

        return std::pair{ std::string{ args[0] }, args.subspan(1) };
    }

    template<Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::string, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }


    template<Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::string, ArgDefinition>::Spec(std::meta::info member) {
        return details_::DefaultSpec<std::string, ArgDefinition, Annotations...>(member);
    }
}
