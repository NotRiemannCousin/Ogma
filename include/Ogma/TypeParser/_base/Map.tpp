#pragma once

#include <array>
#include <format>

namespace Ogma {

    template<class Compare, class Alloc, Arg ArgDefinition>
    template<auto... Annotations>
    auto
    TypeParser<std::map<std::string, std::string, Compare, Alloc>, ArgDefinition>::Parse(
        Map curr, std::string_view, ArgSpan args, std::string_view) -> MapRes {
        if constexpr (Utils::GetAnnotation<Overwrite, Annotations...>())
            curr.clear();

        while (!args.empty() && !Utils::IsFlag(args[0])) {
            const std::string_view token{ args[0] };
            const std::size_t eq{ token.find('=') };

            if (eq == std::string_view::npos)
                return std::unexpected{ std::format("expected a \"key=value\" pair: '{}'", token) };
            if (eq == 0)
                return std::unexpected{ std::format("key shouldn't be empty: '{}'", token) };

            curr.insert_or_assign(std::string{ token.substr(0, eq) }, std::string{ token.substr(eq + 1) });
            args = args.subspan(1);
        }
        if (!args.empty() && args[0] == controlToken) args = args.subspan(1);
        return std::make_pair(std::move(curr), args);
    }

    template<class Compare, class Alloc, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::map<std::string, std::string, Compare, Alloc>, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }


    template<class Compare, class Alloc, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::map<std::string, std::string, Compare, Alloc>, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<std::map<std::string, std::string, Compare, Alloc>, ArgDefinition, Annotations...>(member) };
        spec.metavars = "<KEY=VALUE>...";
        return spec;
    }
}
