// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <variant>

#pragma region Subcommand Dispatch

// ===================================================================================================================
// VariantArg -> subcommands, like in every lib.
// ===================================================================================================================

namespace {
    struct AddOptions {
        [[ =Ogma::Arg{ "file" } ]] std::string file{};
    };

    struct ServeOptions {
        [[ =Ogma::Arg{ "host" } ]] std::string host{};
        [[ =Ogma::Arg{ "port" } ]] int         port{};
    };

    constexpr Ogma::VariantArg::VariantBranch commandCases[]{
        { "add"  , "add a file to the index" },
        { "serve", "start the server"        }
    };

    struct CliOptions {
        [[ =Ogma::VariantArg{ commandCases, 2 } ]] std::variant<AddOptions, ServeOptions> command{};
    };
}

TEST(SubcommandDispatch, BasicParse) {
    // The basic subcommand usage.
    const auto parsed{ Ogma::Parse<CliOptions>({{ "add", "notes.txt" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_TRUE(std::holds_alternative<AddOptions>(parsed->first.command));
    EXPECT_EQ(std::get<AddOptions>(parsed->first.command).file, "notes.txt");
}

TEST(SubcommandDispatch, BasicParseWithFlag) {
    // Same as before but using the flag from a subcommand member.
    const auto parsed{ Ogma::Parse<CliOptions>({{ "add", "--file", "notes.txt" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_TRUE(std::holds_alternative<AddOptions>(parsed->first.command));
    EXPECT_EQ(std::get<AddOptions>(parsed->first.command).file, "notes.txt");
}


TEST(SubcommandDispatch, DifferentPaths) {
    // Each subcommand is tied to a different type from std::variant, each with it own members.
    const auto parsed1{ Ogma::Parse<CliOptions>({{ "serve", "--host", "localhost", "--port", "8080" }}) };
    const auto parsed2{ Ogma::Parse<CliOptions>({{ "add"  , "--file", "notes.txt" }}) };

    ASSERT_TRUE(parsed1.has_value()) << parsed1.error();
    ASSERT_TRUE(std::holds_alternative<ServeOptions>(parsed1->first.command));
    EXPECT_EQ(std::get<ServeOptions>(parsed1->first.command).host, "localhost");
    EXPECT_EQ(std::get<ServeOptions>(parsed1->first.command).port, 8080);

    ASSERT_TRUE(parsed2.has_value()) << parsed2.error();
    ASSERT_TRUE(std::holds_alternative<AddOptions>(parsed2->first.command));
}


TEST(SubcommandDispatch, RejectsFlagSpelling) {
    // The "--" prefix will try to match with a existing option. Given that there is no flag named "--add", the
    // resolution will fail as "unknown option".
    const auto parsed{ Ogma::Parse<CliOptions>({{ "--add" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown option"), std::string::npos);
}

TEST(SubcommandDispatch, FailsUnknownSubcommand) {
    // There is no positional argument and the "--" prefix is missing, so it will be evaluated as a subcommand.
    // Given that there is no subcommand named "remove", the resolution will fail as "unknown subcommand".
    const auto parsed{ Ogma::Parse<CliOptions>({{ "remove" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown subcommand"), std::string::npos);
}

#pragma endregion


#pragma region Subcommand Scope

// ===================================================================================================================
// Entering a branch opens the same kind of scope a nested struct does. The branch owns every following token until
// "-" closes it, and whatever comes after is handed back to the parent. Flags of the parent are read before the
// subcommand, that scope separation thing.
// ===================================================================================================================

namespace {
    // Annotated members are declared before the subcommand, per the structural rule.
    struct VerboseOptions {
        [[ =Ogma::Arg{ "verbose" } ]] bool verbose{};
        [[ =Ogma::VariantArg{ commandCases, 2 } ]] std::variant<AddOptions, ServeOptions> command{};
    };

    constexpr Ogma::VariantArg::VariantBranch echoCases[]{ { "echo", "echo a name" } };

    struct EchoBranch {
        [[ =Ogma::Arg{ "name" } ]] std::string name{};
    };

    struct HomonymOptions {
        [[ =Ogma::Arg{ "name" } ]] std::string name{};
        [[ =Ogma::VariantArg{ echoCases, 1 } ]] std::variant<EchoBranch> command{};
    };
}

TEST(SubcommandScope, ParentFlagsBeforeTheBranch) {
    // Flags of the parent are consumed before the branch opens.
    const auto parsed{ Ogma::Parse<VerboseOptions>({{ "--verbose", "add", "--file", "notes.txt" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_TRUE(parsed->first.verbose);
    ASSERT_TRUE(std::holds_alternative<AddOptions>(parsed->first.command));
}

TEST(SubcommandScope, ParentFlagsInsideBranchAreUnknown) {
    // "--verbose" belongs to the parent, Ogma has no global arguments so it is unknown on the child.
    const auto parsed{ Ogma::Parse<VerboseOptions>({{ "add", "--file", "notes.txt", "--verbose" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown option"), std::string::npos);
}

TEST(SubcommandScope, ClosingTokenReturnsToParent) {
    // "-" closes the branch, "--verbose" is then parsed by the parent.
    const auto parsed{ Ogma::Parse<VerboseOptions>({{ "serve", "--host", "h", "--port", "1", "-", "--verbose" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_TRUE(parsed->first.verbose);
    ASSERT_TRUE(std::holds_alternative<ServeOptions>(parsed->first.command));
    EXPECT_EQ(std::get<ServeOptions>(parsed->first.command).port, 1);
}

TEST(SubcommandScope, NoSecondDispatchAfterClosing) {
    // The command member is already filled, so after "-" there is no way to open the subcommand again.
    const auto parsed{ Ogma::Parse<VerboseOptions>({{ "serve", "--host", "h", "--port", "1", "-", "add" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unexpected argument"), std::string::npos);
}

TEST(SubcommandScope, HomonymFlagAcrossScopes) {
    // Parent and branch declare "name", each occurrence answers to its own level.
    const auto parsed{ Ogma::Parse<HomonymOptions>({{ "--name", "outer", "echo", "--name", "inner" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.name, "outer");
    EXPECT_EQ(std::get<EchoBranch>(parsed->first.command).name, "inner");
}

#pragma endregion


#pragma region Subcommand Order

// ===================================================================================================================
// The subcommand member must be the last annotated member of the struct. Positional members are filled first, so a
// required positional claims case names before the command is reachable. That is because Ogma is required-by-default,
// but std::optional is always here.
// ===================================================================================================================

namespace {
    struct TaggedOptions {
        [[ =Ogma::Arg{ "tag" } ]] std::string tag{};
        [[ =Ogma::VariantArg{ commandCases, 2 } ]] std::variant<AddOptions, ServeOptions> command{};
    };

    struct QuietTagOptions {
        [[ =Ogma::Arg{ "tag" } ]] std::optional<std::string> tag{};
        [[ =Ogma::VariantArg{ commandCases, 2 } ]] std::variant<AddOptions, ServeOptions> command{};
    };
}

TEST(SubcommandOrder, RequiredPositionalClaimsCaseNameFirst) {
    // You need to fill the positional arguments before filling the subcommand. `tag` will match with "add", leaving
    // the `command` member missing.
    const auto parsed{ Ogma::Parse<TaggedOptions>({{ "add" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("missing required subcommand"), std::string::npos);
}

TEST(SubcommandOrder, PositionalFirstThenCommand) {
    // Filling the positional by its position.
    const auto parsed{ Ogma::Parse<TaggedOptions>({{ "hello", "add", "--file", "notes.txt" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.tag, "hello");
    ASSERT_TRUE(std::holds_alternative<AddOptions>(parsed->first.command));
    EXPECT_EQ(std::get<AddOptions>(parsed->first.command).file, "notes.txt");
}


TEST(SubcommandOrder, PositionalUsingNameFirstThenCommand) {
    // Filling the positional by its name.
    const auto parsed{ Ogma::Parse<TaggedOptions>({{ "--tag", "world", "add", "--file", "notes.txt" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.tag, "world");
    ASSERT_TRUE(std::holds_alternative<AddOptions>(parsed->first.command));
    EXPECT_EQ(std::get<AddOptions>(parsed->first.command).file, "notes.txt");
}


TEST(SubcommandOrder, OptionalIsOptional) {
    // std::optional is really optional.
    const auto parsed{ Ogma::Parse<QuietTagOptions>({{ "add", "--file", "notes.txt" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_FALSE(parsed->first.tag.has_value());
    ASSERT_TRUE(std::holds_alternative<AddOptions>(parsed->first.command));
    EXPECT_EQ(std::get<AddOptions>(parsed->first.command).file, "notes.txt");
}

#pragma endregion


#pragma region Subcommand Requirements

// ===================================================================================================================
// Like every non-optional member, a VariantArg member is required and the error message lists every declared case.
// Failures raised while parsing the branch itself propagate with the branch's own context.
// ===================================================================================================================

TEST(SubcommandRequirements, RequiredByDefault) {
    // Without any token the command member is reported as missing, naming every case.
    const auto parsed{ Ogma::Parse<CliOptions>({}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("missing required subcommand"), std::string::npos);
    EXPECT_NE(parsed.error().find("add | serve"), std::string::npos);
}

TEST(SubcommandRequirements, BranchErrorsPropagate) {
    // A branch with missing required members fails through the branch's own error.
    const auto parsed{ Ogma::Parse<CliOptions>({{ "serve" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("missing required argument"), std::string::npos);
    EXPECT_NE(parsed.error().find("--host"), std::string::npos);
}

#pragma endregion