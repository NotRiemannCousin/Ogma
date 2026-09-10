// extract backup.tar doc1.txt doc1.txt doc2.txt -- --timeout 30s
#include <FileTool.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <expected>
#include <print>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <print>

std::expected<int, std::string> CompressCmd::operator()() const {
    std::println("-> Compressing into '{}'", target);
    for (const auto& file : files)
        std::println("   - {}", file);

    if (level)
        std::println("   Level: {}", *level);
    if (configFile)
        std::println("   Config: {}", configFile->string());
    return 0;
}

std::expected<int, std::string> ExtractCmd::operator()() const {
    std::println("-> Extracting '{}'", archive);
    std::println("   Files to extract:");
    for (const auto& file : files)
        std::println("     - {}", file);
    if (timeout)
        std::println("   Timeout: {}s", timeout->count());
    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args{ argv + 1, argv + argc };

    if (args.empty())
        args = { "extract", "backup.tar", "doc1.txt", "doc1.txt", "doc2.txt", "--", "--timeout", "30" };

    return Ogma::ParseAndExecute<App>(args);
}