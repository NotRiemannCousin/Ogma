// ReSharper disable CppDeclaratorNeverUsed
#include <gtest/gtest.h>

#include <string>

#include <Fixtures.hpp>

namespace {
    using HelpFixtures::App;
    using HelpFixtures::CatchHelp;
    using HelpFixtures::MusicsOptions;
    using HelpFixtures::PlainOptions;
}

#pragma region Interception

// ===================================================================================================================
// Testing the "--help" interception mechanism. It short-circuits parsing at the exact scope that sees the token,
// returning the programmatic page that HelpText would render for that same level.
// ===================================================================================================================

TEST(HelpInterception, RootHelpCarriesTheProgrammaticPage) {
    // The root intercepts "--help" and returns the main application page.
    const auto page{ CatchHelp<App>({{ "--help" }}) };

    ASSERT_TRUE(page.has_value()) << "parsing completed without a help request";
    EXPECT_EQ(*page, Ogma::HelpText<App>());
}

TEST(HelpInterception, BranchHelpCarriesTheBranchPage) {
    // A branch intercepts "--help" and returns its own scoped page.
    const auto page{ CatchHelp<App>({{ "dummy", "--tags", "-", "musics", "--help" }}) };

    ASSERT_TRUE(page.has_value()) << "parsing completed without a help request";
    EXPECT_EQ(*page, Ogma::HelpText<MusicsOptions>("demo musics"));
}

TEST(HelpInterception, ClosingTokenHandsTheRequestToTheParent) {
    // Closing a branch with "-" passes the remaining "--help" token back to the parent scope.
    const auto page{ CatchHelp<App>({{ "dummy", "--tags", "-", "musics", "123", "-", "-", "--help" }}) };

    ASSERT_TRUE(page.has_value()) << "parsing completed without a help request";
    EXPECT_EQ(*page, Ogma::HelpText<App>());
}

TEST(HelpInterception, HelpShortCircuitsRequiredValidation) {
    // Intercepting a help request safely bypasses missing required argument errors.
    const auto page{ CatchHelp<PlainOptions>({{ "--help" }}) };

    ASSERT_TRUE(page.has_value()) << "parsing completed without a help request";
    EXPECT_NE(page->find("Usage: PlainOptions"), std::string::npos);
}

#pragma endregion


#pragma region Version

// ===================================================================================================================
// Testing the "--version" interception mechanism. It is generated from the AppInfo annotation at the root level only,
// making it unreachable inside branches.
// ===================================================================================================================

TEST(HelpVersion, RootVersionIsGeneratedFromAppInfo) {
    // The root intercepts "--version" and returns the formatted version string.
    const auto text{ CatchHelp<App>({{ "--version" }}) };

    ASSERT_TRUE(text.has_value()) << "parsing completed without a version request";
    EXPECT_NE(text->find("demo 1.2.3"), std::string::npos);
}

TEST(HelpVersion, VersionIsNotReachableInsideABranch) {
    // Branches do not own the version flag, yielding an unknown option error.
    const auto parsed{ Ogma::Parse<App>({{ "dummy", "--tags", "-", "musics", "--version" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown option"), std::string::npos);
}

TEST(HelpVersion, VersionNeedsAppInfo) {
    // Without AppInfo, the version flag is not generated and yields an unknown option error.
    const auto parsed{ Ogma::Parse<PlainOptions>({{ "--version" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown option"), std::string::npos);
}

#pragma endregion


#pragma region Nested Scope Help

// ===================================================================================================================
// A nested struct opened by its flag is its own scope. The help page shows the full path to it, the parent scope plus
// the typed flag, and matches the programmatic HelpText for the same path.
// ===================================================================================================================

namespace {
    struct NestedInner {
        [[ =Ogma::Arg{ "name", "the name" } ]] std::string name{};
    };

    struct NestedOuter {
        [[ =Ogma::Arg{ "inner" } ]] NestedInner inner{};
        [[ =Ogma::Arg{ "tag"   } ]] std::string  tag{};
    };
}

TEST(NestedScopeHelp, CarriesTheExtendedPath) {
    // Without the threading fix this renders "Usage: NestedOuter --name <NAME>",  the parent's path with the child's
    // members.
    const auto page{ CatchHelp<NestedOuter>({{ "--inner", "--help" }}) };

    ASSERT_TRUE(page.has_value()) << "parsing completed without a help request";
    EXPECT_NE(page->find("Usage: NestedOuter --inner --name <NAME>"), std::string::npos);
}

TEST(NestedScopeHelp, MatchesTheProgrammaticPage) {
    const auto page{ CatchHelp<NestedOuter>({{ "--inner", "--help" }}) };

    ASSERT_TRUE(page.has_value());
    EXPECT_EQ(*page, Ogma::HelpText<NestedInner>("NestedOuter --inner"));
}

#pragma endregion