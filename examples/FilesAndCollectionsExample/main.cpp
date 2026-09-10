/// tmp/ogma-input.txt /tmp/ogma-output.bin --include '*.txt' --include '*.md' --exclude '*.tmp' --compression zstd --manifest /tmp/ogma-manifest.txt
#include <FilesAndCollectionsExample.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <print>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <chrono>
#include <print>

std::expected<int, std::string> FilesApp::operator()() const {
    std::println("input:       {}", input.string());
    std::println("output:      {}", output.string());
    std::println("compression: {}", static_cast<int>(compression));

    std::println("include:");
    for (const auto& pattern : include)
        std::println("  {}", pattern);

    std::println("exclude:");
    for (const auto& pattern : exclude)
        std::println("  {}", pattern);

    if (manifest)
        std::println("manifest:    {}", manifest->string());
    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args{ argv + 1, argv + argc };
    return Ogma::ParseAndExecute<FilesApp>(args);
}
