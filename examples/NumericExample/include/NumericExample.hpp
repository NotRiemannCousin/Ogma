#pragma once

#include <Ogma/Parser.hpp>

#include <cstdint>
#include <expected>
#include <string>

struct [[ =Ogma::AppInfo{
    "numeric-example", "1.0", "Numeric annotation examples."
} ]] NumericApp {
    [[ =Ogma::Arg{ "hex", "The value to decode" }, =Ogma::hexNumber ]]
    std::uint32_t hexValue{};

    [[ =Ogma::Arg{ "mask", "The bitmask to decode" }, =Ogma::binNumber ]]
    std::uint16_t binaryMask{};

    [[ =Ogma::Arg{ "size", "A quantity with SI suffixes" }, =Ogma::siNumber ]]
    std::uint64_t size{};

    [[ =Ogma::Arg{ "retries", "How many attempts before giving up" }, =Ogma::Range{ 0, 10 } ]]
    int retries{};

    [[ =Ogma::Arg{ "ratio", "A value between 0 and 1" }, =Ogma::ratio ]]
    double ratio{ 1.0 };

    std::expected<int, std::string> operator()() const;
};