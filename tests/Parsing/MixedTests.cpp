// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>
#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <variant>
#include <vector>

#pragma region Whole App Scenarios

// ===================================================================================================================
// MixedScenarios -> Features combined in a more realistic invocation.
// ===================================================================================================================

namespace {
    struct TrackInfo {
        [[ =Ogma::Arg{ "title"   } ]] std::string title{};
        [[ =Ogma::Arg{ "seconds" } ]] int seconds{};
    };

    struct MusicsCommand {
        [[ =Ogma::Arg{ "ids" } ]] std::vector<std::string> ids{};
        [[ =Ogma::Arg{ "search" }, =Ogma::boolArg ]] bool search{};
    };

    struct AlbumCommand {
        [[ =Ogma::Arg{ "id" } ]] std::string id{};
    };

    struct PlaylistCommand {
        [[ =Ogma::Arg{ "tracks" } ]] std::vector<TrackInfo> tracks{};
    };

    constexpr Ogma::VariantArg::VariantBranch commands[]{
        { "musics"  , "Download songs."     },
        { "album"   , "Download an album."  },
        { "playlist", "Build a playlist."   }
    };

    struct App {
        [[ =Ogma::Arg{ "quality" } ]] std::optional<std::string> quality{};
        [[ =Ogma::Arg{ "tags"    } ]] std::vector<std::string>   tags{};
        [[ =Ogma::VariantArg{ commands, std::size(commands) } ]]
        std::variant<MusicsCommand, AlbumCommand, PlaylistCommand> command{};
    };
}

TEST(MixedScenarios, ParsesAWholeInvocation) {
    // Root optional, root container, dispatch, and a branch with its own container and bool.
    const auto parsed{ Ogma::Parse<App>({{ "--quality", "best", "--tags", "rock", "mpb", "-",
                                          "musics", "id-one", "id-two", "--search", "true" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_TRUE(parsed->first.quality.has_value());
    EXPECT_EQ(*parsed->first.quality, "best");
    EXPECT_EQ(parsed->first.tags, (std::vector<std::string>{ "rock", "mpb" }));

    ASSERT_TRUE(std::holds_alternative<MusicsCommand>(parsed->first.command));
    const auto& musics{ std::get<MusicsCommand>(parsed->first.command) };
    EXPECT_EQ(musics.ids, (std::vector<std::string>{ "id-one", "id-two" }));
    EXPECT_TRUE(musics.search);
}

TEST(MixedScenarios, RootStateSurvivesTheBranch) {
    // The container closes on "-", the dispatch happens, the branch closes on "-",
    // and the root optional is still reachable afterwards.
    const auto parsed{ Ogma::Parse<App>({{ "--tags", "live", "-", "album", "ALB-1", "-", "--quality", "best" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.tags, (std::vector<std::string>{ "live" }));
    ASSERT_TRUE(std::holds_alternative<AlbumCommand>(parsed->first.command));
    EXPECT_EQ(std::get<AlbumCommand>(parsed->first.command).id, "ALB-1");
    ASSERT_TRUE(parsed->first.quality.has_value());
    EXPECT_EQ(*parsed->first.quality, "best");
}

#pragma endregion


#pragma region Composition Inside Branches

// ===================================================================================================================
// Everyone knows subcommands I don't need to explaint it right here.
// ===================================================================================================================

namespace {
    struct AlbumWithTrackCommand {
        [[ =Ogma::Arg{ "id" } ]] std::string id{};
        [[ =Ogma::Arg{ "track" } ]] TrackInfo track{};
    };

    constexpr Ogma::VariantArg::VariantBranch studioCases[]{
        { "album"   , "Download an album." },
        { "playlist", "Build a playlist."  }
    };

    struct StudioApp {
        [[ =Ogma::Arg{ "quality" } ]] std::optional<std::string> quality{};
        [[ =Ogma::VariantArg{ studioCases, std::size(studioCases) } ]]
        std::variant<AlbumWithTrackCommand, PlaylistCommand> command{};
    };
}

TEST(MixedScenarios, NestedStructInsideABranch) {
    // "--track" opens a scope inside the branch; the first "-" closes it and the second closes the branch itself.
    const auto parsed{ Ogma::Parse<StudioApp>({{ "album", "ALB-1", "--track",
                                                "--title", "Song", "--seconds", "90",
                                                "-", "-", "--quality", "best" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_TRUE(std::holds_alternative<AlbumWithTrackCommand>(parsed->first.command));
    const auto& album{ std::get<AlbumWithTrackCommand>(parsed->first.command) };
    EXPECT_EQ(album.id, "ALB-1");
    EXPECT_EQ(album.track.title, "Song");
    EXPECT_EQ(album.track.seconds, 90);
    ASSERT_TRUE(parsed->first.quality.has_value());
    EXPECT_EQ(*parsed->first.quality, "best");
}

TEST(MixedScenarios, StructContainerInsideABranch) {
    // A named-only struct container inside the branch. Each "--" opens an element, each "-" closes it.
    const auto parsed{ Ogma::Parse<StudioApp>({{ "playlist",
                                                "--tracks", "--", "--title", "T1", "--seconds", "60", "-",
                                                "--tracks", "--", "--title", "T2", "--seconds", "120", "-" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_TRUE(std::holds_alternative<PlaylistCommand>(parsed->first.command));
    const auto& playlist{ std::get<PlaylistCommand>(parsed->first.command) };
    ASSERT_EQ(playlist.tracks.size(), 2u);
    EXPECT_EQ(playlist.tracks[0].title, "T1");
    EXPECT_EQ(playlist.tracks[0].seconds, 60);
    EXPECT_EQ(playlist.tracks[1].title, "T2");
    EXPECT_EQ(playlist.tracks[1].seconds, 120);
}

#pragma endregion