#pragma once

#include <Ogma/Parser.hpp>

#include <chrono>
#include <expected>
#include <optional>
#include <string>

struct [[ =Ogma::AppInfo{
    "time-example", "1.0", "Time point and duration examples."
} ]] TimeApp {
    [[ =Ogma::Arg{ "at", "The moment itself, as an ISO 8601 timestamp" } ]]
    std::chrono::system_clock::time_point at{};

    [[ =Ogma::Arg{ "expires", "When the record expires, as Unix seconds" }, =Ogma::unixSeconds ]]
    std::chrono::system_clock::time_point expires{};

    [[ =Ogma::Arg{ "timeout", "How long the operation may run" }, =Ogma::durationFmt ]]
    std::chrono::milliseconds timeout{};

    [[ =Ogma::Arg{ "poll", "Interval between status checks" } ]]
    std::optional<std::chrono::seconds> poll{};

    std::expected<int, std::string> operator()() const;
};