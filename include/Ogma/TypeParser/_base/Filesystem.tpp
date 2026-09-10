#pragma once

#include <array>
#include <filesystem>
#include <format>

#pragma push_macro("FILESYSTEM_CHECK")
#define FILESYSTEM_CHECK(ANNOT, CHECK, STR) do {                             \
    if constexpr (Utils::GetAnnotation<ANNOT, Annotations...>())             \
        if (!std::filesystem::CHECK(p))                                      \
            return std::unexpected{ std::format(STR ": '{}'", p.string()) }; \
} while (false)

namespace Ogma {
    template<Arg ArgDefinition>
    template<auto... Annotations>
    Result<std::filesystem::path> TypeParser<std::filesystem::path, ArgDefinition>::Parse(
        std::filesystem::path, std::string_view, const ArgSpan args, std::string_view) {
        if (args.empty())
            return std::unexpected{ "expected a path" };

        std::filesystem::path p{ args[0] };

        constexpr auto sla1{ Utils::GetAnnotation<MustExist, Annotations...>() };
        constexpr auto sla2{ Utils::GetAnnotation<FileOnly , Annotations...>() };
        constexpr auto sla3{ Utils::GetAnnotation<DirOnly  , Annotations...>() };

        FILESYSTEM_CHECK(MustExist, exists         , "path does not exist"       );
        FILESYSTEM_CHECK(FileOnly , is_regular_file, "path is not a regular file");
        FILESYSTEM_CHECK(DirOnly  , is_directory   , "path is not a directory"   );

        return std::pair{ std::move(p), args.subspan(1) };
    }

    template<Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::filesystem::path, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }


    template<Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::filesystem::path, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<std::filesystem::path, ArgDefinition, Annotations...>(member) };

        std::string mods{};
        if constexpr (Utils::GetAnnotation<MustExist, Annotations...>()) mods += "[must exist] ";
        if constexpr (Utils::GetAnnotation<FileOnly , Annotations...>()) mods += "[file] ";
        if constexpr (Utils::GetAnnotation<DirOnly  , Annotations...>()) mods += "[dir] ";

        if (!mods.empty()) mods.pop_back();

        spec.modifiers = mods;
        return spec;
    }
}


#pragma pop_macro("FILESYSTEM_CHECK")