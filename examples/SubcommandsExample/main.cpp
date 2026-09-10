// list --format json --limit 50 --all
#include <SubcommandsExample.hpp>

#include <expected>
#include <print>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <print>

std::expected<int, std::string> ListCommand::operator()() const {
    std::println(
        "list: format={}, limit={}, all={}",
        static_cast<int>(format), limit, all ? "yes" : "no");
    return 0;
}

std::expected<int, std::string> SearchCommand::operator()() const {
    std::println("search: {}", query);
    for (const auto& tag : tags)
        std::println("  tag: {}", tag);
    return 0;
}

std::expected<int, std::string> VersionCommand::operator()() const {
    std::println("subcommands-example 1.0.0");
    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args{ argv + 1, argv + argc };

    if (args.empty())
        args = { "list", "--format", "json", "--limit", "50", "--all" };

    return Ogma::ParseAndExecute<App>(args);
}
