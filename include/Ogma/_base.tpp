#pragma once

#include <string>
#include <string_view>
#include <vector>


namespace Ogma {
    template<auto Mem>
    consteval Arg GetArg() {
        return std::meta::extract<Arg>(Utils::GetOf<^^Arg>(Mem));
    }

    template<class T>
    consteval bool HasAppInfo() {
        return !std::meta::annotations_of_with_type(^^T, ^^AppInfo).empty();
    }

    template<class T>
    consteval std::string_view ScopePathOf() {
        if constexpr (HasAppInfo<T>()) {
            const auto annot{ std::meta::constant_of(std::meta::annotations_of_with_type(^^T, ^^AppInfo)[0]) };
            const AppInfo info{ std::meta::extract<AppInfo>(annot) };

            if (const std::string_view name{ info.name }; !name.empty())
                return name;
        }
        return std::string_view{ std::define_static_string(std::meta::identifier_of(^^T)) };
    }


    template<class T>
    std::string RenderVersion() {
        static constexpr std::string_view line{ std::invoke([] {
            const AppInfo info{ std::meta::extract<AppInfo>(
                std::meta::constant_of(std::meta::annotations_of_with_type(^^T, ^^AppInfo)[0])) };

            return std::define_static_string(std::vector{ info.name, info.version } | Utils::concatBySpace);
        }) };
        return std::string{ line };
    }
}
