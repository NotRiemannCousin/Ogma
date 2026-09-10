// --hex 0xff --mask 0b1010 --size 3.3k --retries 3 --ratio 0.75
#include <NumericExample.hpp>

#include <cstdint>
#include <expected>
#include <print>
#include <string_view>
#include <vector>
#include <print>

std::expected<int, std::string> NumericApp::operator()() const {
    std::println("hex:     {}", hexValue);
    std::println("binary:  {}", binaryMask);
    std::println("size:    {}", size);
    std::println("retries: {}", retries);
    std::println("ratio:   {}", ratio);
    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args{ argv + 1, argv + argc };

    if (args.empty())
        args = { "--hex", "0xff", "--mask", "0b1010", "--size", "3.3k", "--retries", "3", "--ratio", "0.75" };

    return Ogma::ParseAndExecute<NumericApp>(args);
}
