#pragma once

#include <Ogma/Parser.hpp>

#include <expected>
#include <string>
#include <vector>

struct MusicsCmd {
    [[ =Ogma::Arg{ "data", "Video IDs, or search terms with --search" } ]]
    std::vector<std::string> ids{};

    [[ =Ogma::Arg{ "search", "Interpret the data as search terms" }, =Ogma::boolArg ]]
    bool search{};

    std::expected<int, std::string> operator()() const;
};