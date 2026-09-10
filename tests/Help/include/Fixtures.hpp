#pragma once

#include <Ogma/Parser.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace HelpFixtures {

    enum class Format { Mp3, Flac, Opus };

    struct MusicsOptions {
        [[ =Ogma::Arg{ .longName = "ids"   , .description = "song ids"           } ]] std::vector<std::string> ids{};
        [[ =Ogma::Arg{ .longName = "search", .description = "filter the listing" }, =Ogma::Fwd<Ogma::boolArg>{} ]]
        std::optional<bool> search{};
    };

    struct AlbumOptions {
        [[ =Ogma::Arg{ "title", "album title" } ]] std::string title{};
    };

    inline constexpr Ogma::VariantArg::VariantBranch commands[]{
        { "musics", "Download songs."    },
        { "album" , "Download an album." }
    };

    struct [[ =Ogma::AppInfo{ "demo", "1.2.3", "does demo things" } ]] App {
        [[ =Ogma::Arg{ "output" , "where to write" } ]] std::string                output{};
        [[ =Ogma::Arg{ "quality", "output quality" } ]] std::optional<std::string> quality{};
        [[ =Ogma::Arg{ "tags"   , "tags to apply"  } ]] std::vector<std::string>   tags{};
        [[ =Ogma::Arg{ "verbose", "chatty output"  } ]] bool                       verbose{};
        [[ =Ogma::Arg{ "format" , "file format"    } ]] std::optional<Format>      format{};
        [[ =Ogma::Arg{ .longName = "secret", .description = "hidden tuning", .hidden = true } ]]
        std::optional<int> secret{};

        [[ =Ogma::VariantArg{ commands, std::size(commands) } ]]
        std::variant<MusicsOptions, AlbumOptions> command{};
    };

    struct PlainOptions {
        [[ =Ogma::Arg{ "value", "the value" } ]] int value{};
    };

    struct ProbeBranch {
        [[ =Ogma::Arg{ "x" } ]] int x{};
    };

    inline constexpr Ogma::VariantArg::VariantBranch probeCases[]{ { "go", "run the probe" } };

    struct UsageProbe {
        [[ =Ogma::Arg{ "out"  , "the output" } ]] std::string              out{};
        [[ =Ogma::Arg{ "opt"  , "an option"  } ]] std::optional<int>       opt{};
        [[ =Ogma::Arg{ "tags" , "the tags"   } ]] std::vector<std::string> tags{};
        [[ =Ogma::Arg{ "flag" , "a switch"   } ]] bool                     flag{};

        // Injected member to validate the `modifiers` field rendering.
        [[ =Ogma::Arg{ "port" , "a network port" }, =Ogma::Range{ 1024, 65535 } ]] int port{};

        [[ =Ogma::VariantArg{ probeCases, std::size(probeCases) } ]] std::variant<ProbeBranch> cmd{};
    };

    template<class T>
    std::optional<std::string> CatchHelp(Ogma::ArgSpan args) {
        try {
            (void) Ogma::Parse<T>(args);
        } catch (const Ogma::HelpRequested& help) {
            return help.text;
        }
        return std::nullopt;
    }

}