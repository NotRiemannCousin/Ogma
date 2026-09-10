#pragma once

#include <Ogma/Parser.hpp>

#include <expected>
#include <string>

struct AlbumCmd {
    [[ =Ogma::Arg{ "data", "Album ID, or album name with --search" } ]]
    std::string album{};

    [[ =Ogma::Arg{ "search", "Interpret the data as a name" }, =Ogma::boolArg ]]
    bool search{};

    std::expected<int, std::string> operator()() const;
};