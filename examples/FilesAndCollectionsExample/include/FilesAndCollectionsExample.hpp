#pragma once

#include <Ogma/Parser.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

enum class [[ =Ogma::caseInsensitive ]] Compression { None, Gzip, Zstd };

struct [[ =Ogma::AppInfo{
    "files-example", "1.0", "Filesystem and collection examples."
} ]] FilesApp {
    [[ =Ogma::Arg{ "input", "File to read" }, =Ogma::mustExist, =Ogma::fileOnly ]]
    std::filesystem::path input{};

    [[ =Ogma::Arg{ "output", "Where to write the result" } ]]
    std::filesystem::path output{};

    [[ =Ogma::Arg{ "include", "Glob patterns of files to include" } ]]
    std::vector<std::string> include{};

    [[ =Ogma::Arg{ "exclude", "Glob patterns of files to skip" } ]]
    std::set<std::string> exclude{};

    [[ =Ogma::Arg{ "compression", "Compression algorithm" } ]]
    Compression compression{ Compression::None };

    [[ =Ogma::Arg{ "manifest", "File to record the processed inputs" }, =Ogma::mustExist, =Ogma::fileOnly ]]
    std::optional<std::filesystem::path> manifest{};

    std::expected<int, std::string> operator()() const;
};