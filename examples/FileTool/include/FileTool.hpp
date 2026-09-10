#pragma once

#include <Ogma/Parser.hpp>

#include <cstddef>
#include <chrono>
#include <expected>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

enum class [[ =Ogma::caseInsensitive ]] Algorithm {
    Zstd [[ =Ogma::Alias{ "zstd" } ]],
    Lz4  [[ =Ogma::Alias{ "lz4"  } ]],
    Gzip [[ =Ogma::Alias{ "gzip" } ]]
};

struct CompressCmd {
    [[ =Ogma::Arg{ "target" } ]]
    std::string target{};

    [[ =Ogma::Arg{ "files" } ]]
    std::vector<std::string> files{};

    [[ =Ogma::Arg{ "level" }, =Ogma::Range{ 1, 9 } ]]
    std::optional<int> level{};

    [[ =Ogma::Arg{ "config" }, =Ogma::mustExist, =Ogma::fileOnly ]]
    std::optional<std::filesystem::path> configFile{};

    std::expected<int, std::string> operator()() const;
};

struct ExtractCmd {
    [[ =Ogma::Arg{ "archive" } ]]
    std::string archive{};

    [[ =Ogma::Arg{ "files" } ]]
    std::set<std::string> files{};

    [[ =Ogma::Arg{ "timeout" } ]]
    std::optional<std::chrono::seconds> timeout{};

    std::expected<int, std::string> operator()() const;
};

inline constexpr Ogma::VariantArg::VariantBranch fileToolCommands[]{
    { "compress", "Compress files." },
    { "extract" , "Extract files." }
};

struct [[ =Ogma::AppInfo{ "filetool", "1.0.0", "A file archiver." } ]] App {
    [[ =Ogma::VariantArg{ fileToolCommands, std::size(fileToolCommands) } ]]
    std::variant<CompressCmd, ExtractCmd> command;
};


