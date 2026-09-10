#pragma once

#include <array>
#include <format>

namespace Ogma {

    template<Arg ArgDefinition>
    template<auto... Annotations>
    Result<bool> TypeParser<bool, ArgDefinition>::Parse(bool, std::string_view flag, ArgSpan args, std::string_view) {
        if constexpr (Utils::GetAnnotation<BoolArg, Annotations...>()) {
            if (args.empty())
                return std::unexpected{ "expected 'true' or 'false'" };
            if (args[0] == "true" ) return std::make_pair(true , args.subspan(1));
            if (args[0] == "false") return std::make_pair(false, args.subspan(1));
            return std::unexpected{ std::format("expected 'true' or 'false', got '{}'", args[0]) };
        } else {
            constexpr Arg ann{ ArgDefinition };

            if (flag == std::format("--{}", ann.longName))
                return std::make_pair(true, args);
            if (flag == std::format("--no-{}", ann.longName))
                return std::make_pair(false, args);
            return std::unexpected{ std::format("invalid flag: '{}'", flag) };
        }
    }

    template<Arg ArgDefinition>
    template<auto... Annotations>
    constexpr ArgSpan TypeParser<bool, ArgDefinition>::ExpectedFlags() {
        if constexpr (Utils::GetAnnotation<BoolArg, Annotations...>()) {
            static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
            return list;
        } else {
            static constexpr ConstString name{ ArgDefinition.longName };
            static constexpr const char* noName{ std::define_static_string(std::string{ "no-" } + name) };
            static constexpr std::array<std::string_view, 2> list{ name, noName };
            return list;
        }
    }

    template<Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<bool, ArgDefinition>::Spec(std::meta::info member) {
        ValueSpec spec{ details_::DefaultSpec<bool, ArgDefinition, Annotations...>(member) };

        if (Utils::GetAnnotation<BoolArg, Annotations...>())
            spec.metavars = "<TRUE|FALSE>";
        else {
            spec.metavars = "";
            spec.flagList = std::string{ controlToken } + "[no-]" + std::string{ ArgDefinition.longName };
        }
        return spec;
    }
    
}
