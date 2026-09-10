#pragma once

#include <Ogma/Parser.hpp>

#include <expected>
#include <string>

struct PlaylistCmd {
    [[ =Ogma::Arg{ "data", "Playlist ID, or playlist name with --search" } ]]
    std::string playlist{};

    [[ =Ogma::Arg{ "search"     , "Interpret the data as a name"       }, =Ogma::boolArg ]] bool search{};
    [[ =Ogma::Arg{ "keep-order" , "Keep the playlist's original order" }, =Ogma::boolArg ]] bool keepPlaylistOrder{};

    std::expected<int, std::string> operator()() const;
};