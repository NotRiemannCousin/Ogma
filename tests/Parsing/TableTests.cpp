// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>

#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#pragma region Compile-Time Tables

// ===================================================================================================================
// consteval asserts because I can't test it in another way.
// ===================================================================================================================

namespace {
    struct MusicsCommand {
        [[ =Ogma::Arg{ "query" } ]] std::string query{};
    };

    struct AlbumCommand {
        [[ =Ogma::Arg{ "title" } ]] std::string title{};
    };

    constexpr Ogma::VariantArg::VariantBranch commands[]{
        { "musics", "search musics" },
        { "album" , "search albums" }
    };

    struct App {
        [[ =Ogma::Arg{ "quality" } ]] std::optional<std::string> quality{};
        [[ =Ogma::VariantArg{ commands, std::size(commands) } ]] std::variant<MusicsCommand, AlbumCommand> command;
    };

    // Every spelling of a raw bool is a table entry, the synthetic "no-" included.
    struct SwitchOptions {
        [[ =Ogma::Arg{ "switch" } ]] bool switch_{};
    };

    // Positional-capable members keep their named spelling but never the "" marker.
    struct PairOptions {
        [[ =Ogma::Arg{ "first"  } ]] std::string first{};
        [[ =Ogma::Arg{ "second" } ]] std::string second{};
    };

    // The positional queue follows declaration order, subcommand at the end.
    struct OrderedOptions {
        [[ =Ogma::Arg{ "first"  } ]] std::string first{};
        [[ =Ogma::Arg{ "second" } ]] std::string second{};

        [[ =Ogma::VariantArg{ commands, std::size(commands) } ]]
        std::variant<MusicsCommand, AlbumCommand> command;
    };
}

using Ogma::details_::PositionalArgChoices;
using Ogma::details_::NamedArgChoices;

using std::string_view_literals::operator ""sv;

// App -> optional members are named-only, so the subcommand is the only positional entry.
static_assert(PositionalArgChoices<App>().size() == 1);
static_assert(NamedArgChoices     <App>().size() == 1);
static_assert(PositionalArgChoices<App>()[0].identifier == "command"sv);
static_assert(NamedArgChoices     <App>()[0].first      == "quality"sv);

// Raw bool -> both spellings enter the table, lexicographically sorted — "no-switch" before "switch".
static_assert(NamedArgChoices<SwitchOptions>().size() == 2);
static_assert(NamedArgChoices<SwitchOptions>()[0].first == "no-switch"sv);
static_assert(NamedArgChoices<SwitchOptions>()[1].first == "switch"sv);

// Positional-capable strings -> named table keeps only the spellings, never the "" positional marker.
static_assert(NamedArgChoices<PairOptions>().size() == 2);
static_assert(NamedArgChoices<PairOptions>()[0].first == "first"sv);
static_assert(NamedArgChoices<PairOptions>()[1].first == "second"sv);
static_assert(PositionalArgChoices<PairOptions>().size() == 2);

// Positional queue -> declaration order, subcommand last (the structural rule the builder enforces).
static_assert(PositionalArgChoices<OrderedOptions>().size() == 3);
static_assert(PositionalArgChoices<OrderedOptions>()[0].identifier == "first"sv);
static_assert(PositionalArgChoices<OrderedOptions>()[1].identifier == "second"sv);
static_assert(PositionalArgChoices<OrderedOptions>()[2].identifier == "command"sv);

#pragma endregion