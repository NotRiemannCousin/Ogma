#pragma once

#include <AlbumCmd.hpp>
#include <ArtistCmd.hpp>
#include <AudioFormat.hpp>
#include <MusicsCmd.hpp>
#include <PlaylistCmd.hpp>

#include <Ogma/Parser.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>

inline constexpr Ogma::VariantArg::VariantBranch sonusCommands[]{
    { "musics"  , "Download multiple songs."    },
    { "album"   , "Download a full album."      },
    { "artist"  , "Download an artist catalog." },
    { "playlist", "Download a playlist."        }
};

struct [[ =Ogma::AppInfo{ "sonus", "0.1.0", "A YouTube Music Downloader." } ]] App {

    [[ =Ogma::Arg{ "quality", "Preferred audio quality, as reported by the source" } ]]
    std::optional<std::string> quality{};

    [[ =Ogma::Arg{ "format", "Container format of the downloaded files" } ]]
    std::optional<AudioFormat> format{};

    [[ =Ogma::Arg{ "output", "Directory to place the downloads" }, =Ogma::dirOnly ]]
    std::optional<std::filesystem::path> outputDirectory{};

    [[ =Ogma::VariantArg{ sonusCommands, std::size(sonusCommands) } ]]
    std::variant<MusicsCmd, AlbumCmd, ArtistCmd, PlaylistCmd> command;

    std::expected<int, std::string> operator()() const;
};