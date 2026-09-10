// 2026-09-08T12:30:00Z --expires 1799481600 --timeout 2h30m --poll 5
#include <TimeExample.hpp>

#include <chrono>
#include <expected>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <vector>
#include <print>

std::expected<int, std::string> TimeApp::operator()() const {
    const auto atSeconds{ std::chrono::duration_cast<std::chrono::seconds>(
        at.time_since_epoch()).count() };
    const auto expiresSeconds{ std::chrono::duration_cast<std::chrono::seconds>(
        expires.time_since_epoch()).count() };

    std::println("at:      {} seconds since epoch", atSeconds);
    std::println("expires: {} seconds since epoch", expiresSeconds);
    std::println("timeout: {} ms", timeout.count());

    if (poll) std::println("poll:    {} s", poll->count());
    else      std::println("poll:    disabled");

    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args{ argv + 1, argv + argc };
    return Ogma::ParseAndExecute<TimeApp>(args);
}
