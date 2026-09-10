#pragma once

#include <Ogma/Parser.hpp>

#include <cstddef>
#include <expected>
#include <string>
#include <variant>
#include <vector>

enum class OutputFormat {
    Text [[ =Ogma::Alias{ "text" } ]],
    Json [[ =Ogma::Alias{ "json" } ]],
    Csv [[ =Ogma::Alias{ "csv" } ]]
};

struct ListCommand {
    [[ =Ogma::Arg{ "format", "Output format" } ]]
    OutputFormat format{ OutputFormat::Text };

    [[ =Ogma::Arg{ "limit", "Maximum records to list" }, =Ogma::Range{ 1, 1000 } ]]
    int limit{ 20 };

    [[ =Ogma::Arg{ "all", "List every record, ignoring the limit" }, =Ogma::boolArg ]]
    bool all{};

    std::expected<int, std::string> operator()() const;
};

struct SearchCommand {
    [[ =Ogma::Arg{ "query", "Text to look for" } ]]
    std::string query{};

    [[ =Ogma::Arg{ "tag", "Restrict the search to these tags" } ]]
    std::vector<std::string> tags{};

    std::expected<int, std::string> operator()() const;
};

struct VersionCommand {
    std::expected<int, std::string> operator()() const;
};

inline constexpr Ogma::VariantArg::VariantBranch subcommandBranches[]{
    { "list", "List records." },
    { "search", "Search records." },
    { "version", "Print the version." }
};

struct [[ =Ogma::AppInfo{
    "subcommands-example", "1.0.0", "Variant-backed subcommands."
} ]] App {
    [[ =Ogma::VariantArg{ subcommandBranches, std::size(subcommandBranches) } ]]
    std::variant<ListCommand, SearchCommand, VersionCommand> command;
};