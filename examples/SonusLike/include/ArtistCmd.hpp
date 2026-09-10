#pragma once

#include <Ogma/Parser.hpp>

#include <expected>
#include <string>

struct ArtistCmd {
    [[ =Ogma::Arg{ "data", "Artist ID, or artist name with --search" } ]]
    std::string artist{};

    [[ =Ogma::Arg{ "search" , "Interpret the data as a name"    }, =Ogma::boolArg ]] bool search{};
    [[ =Ogma::Arg{ "videos" , "Also download the artist videos" }, =Ogma::boolArg ]] bool downloadVideos{};
    [[ =Ogma::Arg{ "singles", "Also download singles and EPs"   }, =Ogma::boolArg ]] bool downloadSinglesAndEPs{};
    [[ =Ogma::Arg{ "live"   , "Also download live performances" }, =Ogma::boolArg ]] bool downloadLivePerformances{};

    std::expected<int, std::string> operator()() const;
};