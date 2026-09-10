// ReSharper disable CppDeclaratorNeverUsed
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>

#include <Fixtures.hpp>

namespace {
    using HelpFixtures::App;
    using HelpFixtures::MusicsOptions;
    using HelpFixtures::PlainOptions;
    using HelpFixtures::UsageProbe;

    struct ModifiersProbe {
        [[ =Ogma::Arg{ "hex", "A hex number" }, =Ogma::hexNumber ]]
        int hexNum{};

        [[ =Ogma::Arg{ "float", "A float" }, =Ogma::allowSpecialValues ]]
        double floatNum{};

        [[ =Ogma::Arg{ "file", "A file" }, =Ogma::mustExist, =Ogma::fileOnly ]]
        std::filesystem::path file{};

        [[ =Ogma::Arg{ "dir", "A dir" }, =Ogma::dirOnly ]]
        std::filesystem::path dir{};

        [[ =Ogma::Arg{ "time", "A duration" }, =Ogma::durationFmt ]]
        std::chrono::milliseconds time{};

        [[ =Ogma::Arg{ "timestamp", "A time point" }, =Ogma::unixSeconds ]]
        std::chrono::system_clock::time_point timestamp{};

        [[ =Ogma::Arg{ "config", "A map" } ]]
        std::map<std::string, std::string> config{};
    };
}

#pragma region Help Text Layout

// ===================================================================================================================
// Validates the canonical help page anatomy: about line, usage, commands, positional arguments, named options and the
// automatic entries under Other. The usage grammar: bare fragment = required with fixed position, (parenthesised) =
// required with free position, [bracketed] = omittable, <COMMAND> = subcommand dispatch.
// ===================================================================================================================

TEST(HelpTextLayout, HasTheCanonicalSectionsInOrder) {
    // All canonical sections render in the correct hierarchical order.
    const auto page{ Ogma::HelpText<App>() };

    EXPECT_NE(page.find("demo 1.2.3 - does demo things"), std::string::npos);
    EXPECT_NE(page.find("Usage: demo")                  , std::string::npos);
    EXPECT_NE(page.find("Commands:")                    , std::string::npos);
    EXPECT_NE(page.find("Positional arguments:")        , std::string::npos);
    EXPECT_NE(page.find("Named options:")               , std::string::npos);
    EXPECT_NE(page.find("Other:")                       , std::string::npos);

    EXPECT_LT(page.find("Usage: demo")          , page.find("Commands:"));
    EXPECT_LT(page.find("Commands:")            , page.find("Positional arguments:"));
    EXPECT_LT(page.find("Positional arguments:"), page.find("Named options:"));
    EXPECT_LT(page.find("Named options:")       , page.find("Other:"));
}

TEST(HelpTextLayout, CommandsSectionListsCasesWithTheirDescriptions) {
    // The commands section lists each subcommand case alongside its description.
    const auto page{ Ogma::HelpText<App>() };

    EXPECT_NE(page.find("Download songs.")   , std::string::npos);
    EXPECT_NE(page.find("Download an album."), std::string::npos);
    EXPECT_LT(page.find("musics"), page.find("album"));
}

TEST(HelpTextLayout, PositionalArgumentsSectionShowsBothFaces) {
    // Positional rows lead with the named face and carry the metavar; the bare
    // slot is the metavar itself, and the usage line shows the named form
    // (a flag-filled member leaves the positional queue).
    const auto page{ Ogma::HelpText<App>() };

    EXPECT_NE(page.find("--output <OUTPUT>") , std::string::npos);
    EXPECT_NE(page.find("--tags <TAGS>...")  , std::string::npos);
    EXPECT_NE(page.find("where to write")    , std::string::npos);
    EXPECT_NE(page.find("tags to apply")     , std::string::npos);
}

TEST(HelpTextLayout, NamedOptionsSectionKeepsDeclarationOrder) {
    // Named-only rows keep declaration order; descriptions only exist in the
    // section rows, so they pin the order unambiguously.
    const auto page{ Ogma::HelpText<App>() };

    EXPECT_NE(page.find("[--quality <QUALITY>]")   , std::string::npos);
    EXPECT_NE(page.find("--[no-]verbose")          , std::string::npos);
    EXPECT_NE(page.find("[--format Flac|Mp3|Opus]"), std::string::npos);

    EXPECT_LT(page.find("output quality"), page.find("chatty output"));
    EXPECT_LT(page.find("chatty output"), page.find("file format"));
}

TEST(HelpTextLayout, RowsShowModifiers) {
    // Value constraints and formats are appended to the member's description.
    const auto page{ Ogma::HelpText<UsageProbe>() };

    EXPECT_NE(page.find("--port <PORT>")                     , std::string::npos);
    EXPECT_NE(page.find("a network port [range: 1024..65535]"), std::string::npos);
}

TEST(HelpTextLayout, AutoHelpEntryIsListed) {
    // The automatic --help entry is always advertised, spelt exactly as the
    // parser accepts it.
    const auto page{ Ogma::HelpText<App>() };

    EXPECT_NE(page.find("--help")  , std::string::npos);
    EXPECT_NE(page.find("Print help"), std::string::npos);
}

TEST(HelpTextLayout, AutoVersionEntryOnlyWithAppInfo) {
    // --version is only advertised when AppInfo is declared, and Named options
    // stays hidden when a scope declares no named-only members.
    const auto page{ Ogma::HelpText<App>() };
    const auto plain{ Ogma::HelpText<PlainOptions>() };

    EXPECT_NE(page.find("--version")    , std::string::npos);
    EXPECT_NE(page.find("Print version"), std::string::npos);
    EXPECT_EQ(plain.find("--version")   , std::string::npos);
    EXPECT_EQ(plain.find("Named options:"), std::string::npos);
}

TEST(HelpTextLayout, OtherSectionDocumentsTheClosingToken) {
    // The closing token is documented as an automatic entry, after --help.
    const auto page{ Ogma::HelpText<App>() };

    EXPECT_NE(page.find("Close the current scope"), std::string::npos);
    EXPECT_LT(page.find("--help"), page.find("Close the current scope"));
}

TEST(HelpTextLayout, FallsBackToTheStructIdentifierWithoutAppInfo) {
    // No AppInfo: no about line, and the usage names the struct itself.
    const auto page{ Ogma::HelpText<PlainOptions>() };

    EXPECT_TRUE(page.starts_with("Usage: PlainOptions --value <VALUE>"));
    EXPECT_EQ(page.find("PlainOptions - "), std::string::npos);
}

TEST(HelpTextLayout, HiddenMembersNeverAppear) {
    // Members annotated as hidden are completely omitted from the help page.
    const auto page{ Ogma::HelpText<App>() };

    EXPECT_EQ(page.find("secret")       , std::string::npos);
    EXPECT_EQ(page.find("hidden tuning"), std::string::npos);
}

#pragma endregion


#pragma region Rich Modifiers & Metavars

// ===================================================================================================================
// Verifies the extraction and rendering of semantic meta variables and validation constraints (modifiers).
// ===================================================================================================================

TEST(HelpModifiers, NumericFormats) {
    // Numeric types render their specific format constraints.
    const auto page{ Ogma::HelpText<ModifiersProbe>() };

    EXPECT_NE(page.find("A hex number [format: hex]") , std::string::npos);
    EXPECT_NE(page.find("A float [allow infinity]")   , std::string::npos);
}

TEST(HelpModifiers, FilesystemConstraints) {
    // Filesystem types render their existence and structural validation modifiers.
    const auto page{ Ogma::HelpText<ModifiersProbe>() };

    EXPECT_NE(page.find("A file [must exist] [file]"), std::string::npos);
    EXPECT_NE(page.find("A dir [dir]")              , std::string::npos);
}

TEST(HelpModifiers, ChronoFormats) {
    // Chrono types render their specific time formats.
    const auto page{ Ogma::HelpText<ModifiersProbe>() };

    EXPECT_NE(page.find("A duration [format: human time]")   , std::string::npos);
    EXPECT_NE(page.find("A time point [format: unix seconds]"), std::string::npos);
}

TEST(HelpModifiers, MapForcesKeyValueMetavariable) {
    // Maps forcefully override their metavariable to the canonical <KEY=VALUE>... format.
    const auto page{ Ogma::HelpText<ModifiersProbe>() };

    EXPECT_NE(page.find("--config <KEY=VALUE>..."), std::string::npos);
}

#pragma endregion


#pragma region Usage Fragments

// ===================================================================================================================
// Validates the assembly of the usage line: required fragments first (flags included, since a flag-filled member
// leaves the positional queue), then the optional group, then <COMMAND>. Above the threshold the optional group
// collapses into [OPTIONS].
// ===================================================================================================================

TEST(UsageFragments, TheWholeUsageLineFollowsTheConvention) {
    // Required group in declaration order, then optionals bracketed, then <COMMAND>.
    const auto page{ Ogma::HelpText<UsageProbe>() };

    EXPECT_NE(page.find(
        "Usage: UsageProbe --out <OUT> --tags <TAGS>... (--flag) --port <PORT> [--opt <OPT>] <COMMAND>"),
        std::string::npos);
}

TEST(UsageFragments, TheRootUsageLineFollowsTheConvention) {
    // The root usage line assembles every fragment including the trailing <COMMAND>.
    const auto page{ Ogma::HelpText<App>() };

    EXPECT_NE(page.find(
        "Usage: demo --output <OUTPUT> --tags <TAGS>... (--verbose) [--quality <QUALITY>] [--format Flac|Mp3|Opus] <COMMAND>"),
        std::string::npos);
}

TEST(UsageFragments, TogglesAreGroupedAsNamedOnly) {
    // A raw bool is named-only: the parenthesised usage form (free position),
    // and both spellings documented in the row.
    const auto page{ Ogma::HelpText<UsageProbe>() };

    EXPECT_NE(page.find("--[no-]flag"), std::string::npos);
    EXPECT_NE(page.find("(--flag)")   , std::string::npos);
    EXPECT_NE(page.find("a switch")   , std::string::npos);
}

TEST(UsageFragments, OptionalsCollapsePastTheThreshold) {
    // Five optional fragments render in full; six exceed the threshold and
    // collapse into [OPTIONS]. The required group never collapses.
    struct FiveOptionals {
        [[=Ogma::Arg{ "a" }]] std::optional<int> a{};
        [[=Ogma::Arg{ "b" }]] std::optional<int> b{};
        [[=Ogma::Arg{ "c" }]] std::optional<int> c{};
        [[=Ogma::Arg{ "d" }]] std::optional<int> d{};
        [[=Ogma::Arg{ "e" }]] std::optional<int> e{};
    };

    struct SixOptionals {
        [[=Ogma::Arg{ "a" }]] std::optional<int> a{};
        [[=Ogma::Arg{ "b" }]] std::optional<int> b{};
        [[=Ogma::Arg{ "c" }]] std::optional<int> c{};
        [[=Ogma::Arg{ "d" }]] std::optional<int> d{};
        [[=Ogma::Arg{ "e" }]] std::optional<int> e{};
        [[=Ogma::Arg{ "f" }]] std::optional<int> f{};
    };

    EXPECT_NE(Ogma::HelpText<FiveOptionals>().find(
        "Usage: FiveOptionals [--a <A>] [--b <B>] [--c <C>] [--d <D>] [--e <E>]"), std::string::npos);
    EXPECT_NE(Ogma::HelpText<SixOptionals>().find(
        "Usage: SixOptionals [OPTIONS]"), std::string::npos);
}

#pragma endregion


#pragma region Branch Pages

// ===================================================================================================================
// Validates the help pages generated for inner branch scopes.
// ===================================================================================================================

TEST(BranchPages, TheUsageLineCarriesTheScopePath) {
    // The branch usage line prepends the parent scope path.
    const auto page{ Ogma::HelpText<MusicsOptions>("demo musics") };

    EXPECT_NE(page.find("Usage: demo musics --ids <IDS>... [--search <TRUE|FALSE>]"),
              std::string::npos);
}

TEST(BranchPages, PositionalsLandInTheArgumentsSection) {
    // Branch positional members are routed to the positional arguments section,
    // with their named face documented in the same row.
    const auto page{ Ogma::HelpText<MusicsOptions>("demo musics") };

    EXPECT_NE(page.find("Positional arguments:"), std::string::npos);
    EXPECT_NE(page.find("--ids <IDS>...")       , std::string::npos);
    EXPECT_NE(page.find("song ids")             , std::string::npos);
}

TEST(BranchPages, NoCommandsAndNoAbout) {
    // Leaf branches do not render an about line or a commands section.
    const auto page{ Ogma::HelpText<MusicsOptions>("demo musics") };

    EXPECT_EQ(page.find("Commands:")      , std::string::npos);
    EXPECT_EQ(page.find("Download songs."), std::string::npos);
}

TEST(BranchPages, HelpEntryButNoVersion) {
    // Branches answer to help requests but never advertise the root-only version flag.
    const auto page{ Ogma::HelpText<MusicsOptions>("demo musics") };

    EXPECT_NE(page.find("--help")   , std::string::npos);
    EXPECT_EQ(page.find("--version"), std::string::npos);
}

#pragma endregion