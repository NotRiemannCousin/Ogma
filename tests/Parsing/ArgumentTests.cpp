// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#pragma region Named Members

// ===================================================================================================================
// StructuralNaming -> Arguments that need to be specified by their flag.
// ===================================================================================================================

namespace {
    struct GreeterOptions {
        [[ =Ogma::Arg{ "name" } ]] std::string name{};
        [[ =Ogma::Arg{ "bind" } ]] bool        bind{};
    };
}

TEST(StructuralNaming, ParsesNamedMembers) {
    // Named flags fill their members in place.
    const auto parsed{ Ogma::Parse<GreeterOptions>({{ "--name", "abracadabra", "--bind" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.name, "abracadabra");
    EXPECT_TRUE(parsed->first.bind);
}


TEST(StructuralNaming, RejectsUnknownOption) {
    // A flag no member recognises.
    const auto parsed{ Ogma::Parse<GreeterOptions>({{ "--name", "x", "--nonsense" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown option"), std::string::npos);
}

#pragma endregion


#pragma region Positional Members

// ===================================================================================================================
// Positional -> The token fills the first still-unfilled positional member in declaration order. A member already
// filled by its flag is skipped.
// ===================================================================================================================

namespace {
    struct PairOptions {
        [[ =Ogma::Arg{ "first"  } ]] std::string first{};
        [[ =Ogma::Arg{ "second" } ]] std::string second{};
    };
}

TEST(PositionalMatch, FillingPositionalFields) {
    // No flag needed.
    const auto parsed{ Ogma::Parse<PairOptions>({{ "one", "two" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.first , "one");
    EXPECT_EQ(parsed->first.second, "two");
}

TEST(PositionalMatch, NamedAssignmentSkipsAMemberInThePositionalOrder) {
    // "--second" match the second member, so the next token goes to the next unfilled one ("first").
    const auto parsed{ Ogma::Parse<PairOptions>({{ "--second", "named", "positional" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.first , "positional");
    EXPECT_EQ(parsed->first.second, "named");
}

TEST(PositionalMatch, RejectsLeftoverBareToken) {
    // Nothing left to fill.
    const auto parsed{ Ogma::Parse<GreeterOptions>({{ "abracadabra", "leftover" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unexpected argument"), std::string::npos);
}

#pragma endregion




#pragma region Nested Structural Entry

namespace {
    struct Inner {
        [[ =Ogma::Arg{ "name"  } ]] std::string name{};
        [[ =Ogma::Arg{ "count" } ]] int         count{};
    };

    struct Outer {
        [[ =Ogma::Arg{ "inner" } ]] Inner       inner{};
        [[ =Ogma::Arg{ "tag"   } ]] std::string tag{};
    };
}

TEST(NestedStructural, NamedNestedStruct) {
    // Structural members are named-only, you need to write their flag to set it.
    const auto parsed{ Ogma::Parse<Outer>({{
        "--inner", "Ada", "--count", "5", "-",
        "hello"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.inner.name , "Ada");
    EXPECT_EQ(parsed->first.inner.count, 5);
    EXPECT_EQ(parsed->first.tag        , "hello");
}
TEST(NestedStructural, FailsPositionalNestedStruct) {
    // Structural members are named-only, you need to write their flag to set it. It will fail with unknown "--name".
    const auto parsed{ Ogma::Parse<Outer>({{
        "--name", "Ada", "--count", "5", "-",
        "hello"
    }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown option"), std::string::npos);
}

#pragma endregion




#pragma region Structural Scope

// ===================================================================================================================
// Instead of "a flat list of tokens" this library is scope-based. "-" is the official closing token of this library,
// it will exit any scope, from containers to nested structural members.
// ===================================================================================================================

namespace {
    struct HomonymChild {
        [[ =Ogma::Arg{ "name" } ]] std::string name{};
    };

    struct HomonymParent {
        [[ =Ogma::Arg{ "child" } ]] HomonymChild child{};
        [[ =Ogma::Arg{ "name"  } ]] std::string  name{};
    };

    struct TwinOptions {
        [[ =Ogma::Arg{ "alpha" } ]] Inner alpha{};
        [[ =Ogma::Arg{ "beta"  } ]] Inner beta{};
    };
}

TEST(StructuralScope, ClosingBoundaryIsOptionalWhenArgsEnd) {
    // The closing token is optional when the arguments list is over.
    const auto parsed{ Ogma::Parse<Outer>({{
        "hello",
        "--inner", "--name", "Ada", "--count", "5"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.tag        , "hello");
    EXPECT_EQ(parsed->first.inner.name , "Ada");
    EXPECT_EQ(parsed->first.inner.count, 5);
}


TEST(StructuralScope, ScopeSegregation) {
    // "--tag" inside the child is unknown there, not handed back to the parent.
    const auto parsed{ Ogma::Parse<Outer>({{ "--name", "Ada", "--count", "5", "--tag", "tail" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown option"), std::string::npos);
}

TEST(StructuralScope, ScopeIsolation) {
    // Parent and child both declare "name". Each occurrence answers to its own level.
    const auto parsed{ Ogma::Parse<HomonymParent>({{ "--name", "outer", "--child", "--name", "inner" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.name       , "outer");
    EXPECT_EQ(parsed->first.child.name , "inner");
}


TEST(StructuralScope, FailsExitingScopeIsRequiredToOpenNewMember) {
    // Needs "-" to close "alpha" before opening "beta".
    const auto parsed{ Ogma::Parse<TwinOptions>({{
        "--alpha", "--name", "one", "--count", "1",
        "--beta" , "--name", "two", "--count", "2"
    }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown option"), std::string::npos);
}

TEST(StructuralScope, ExitingScopeIsRequiredToOpenNewMember) {
    // Needs "-" to close "alpha" before opening "beta".
    const auto parsed{ Ogma::Parse<TwinOptions>({{
        "--alpha", "--name", "one", "--count", "1", "-",
        "--beta" , "--name", "two", "--count", "2"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.alpha.name , "one");
    EXPECT_EQ(parsed->first.alpha.count, 1);
    EXPECT_EQ(parsed->first.beta.name  , "two");
    EXPECT_EQ(parsed->first.beta.count , 2);
}

#pragma endregion


#pragma region Repeated Structural Members

// ===================================================================================================================
// OrdinaryArguments -> Struct is a named-only argument, so vector<Struct> will take one element per "--"-opened group,
// each closed by its own "-". Delimiter mechanics live in ContainerTests.
// ===================================================================================================================

namespace {
    struct PointOptions {
        [[ =Ogma::Arg{ "x" } ]] int x{};
        [[ =Ogma::Arg{ "y" } ]] int y{};
    };

    struct TrackOptions {
        [[ =Ogma::Arg{ "points" } ]] std::vector<PointOptions> points{};
        [[ =Ogma::Arg{ "label"  } ]] std::string               label{};
    };
}

TEST(StructsInContainers, RepeatedStructuralMemberTakesOneElementPerGroup) {
    // One element per group, parent flag filled where it is unambiguous.
    const auto parsed{ Ogma::Parse<TrackOptions>({{
        "--label", "end",
        "--points", "--", "--x", "1", "--y", "2", "-",
        "--points", "--", "--x", "3", "--y", "4", "-",
        "-"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_EQ(parsed->first.points.size(), 2u);
    EXPECT_EQ(parsed->first.points[0].x, 1);
    EXPECT_EQ(parsed->first.points[0].y, 2);
    EXPECT_EQ(parsed->first.points[1].x, 3);
    EXPECT_EQ(parsed->first.points[1].y, 4);
    EXPECT_EQ(parsed->first.label, "end");
}

TEST(StructsInContainers, RequiresRepeatedStructuralMember) {
    // Like every non-optional field, the member is required even when it would end up empty.
    const auto parsed{ Ogma::Parse<TrackOptions>({{ "--label", "end" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("missing required argument"), std::string::npos);
}

#pragma endregion


#pragma region Boolean Members

// ===================================================================================================================
// A bool without boolArg is an on/off switch, "--name" sets true and "--no-name" sets false, consuming nothing. With
// boolArg it takes an explicit "true"/"false" value instead.
// ===================================================================================================================

namespace {
    struct SwitchBoolOptions {
        [[ =Ogma::Arg{ "flag" } ]] bool flag{};
    };

    struct ExplicitBoolOptions {
        [[ =Ogma::Arg{ "flag" }, =Ogma::boolArg ]] bool flag{};
    };
}

TEST(BooleanMembers, SwitchUsesPositiveAndNegativeSpellings) {
    // The switch has two spellings and consumes nothing.
    const auto parsedOn { Ogma::Parse<SwitchBoolOptions>({{ "--flag"     }}) };
    const auto parsedOff{ Ogma::Parse<SwitchBoolOptions>({{ "--no-flag" }}) };

    ASSERT_TRUE(parsedOn.has_value())  << parsedOn.error();
    ASSERT_TRUE(parsedOff.has_value()) << parsedOff.error();
    EXPECT_TRUE(parsedOn->first.flag);
    EXPECT_FALSE(parsedOff->first.flag);
}

TEST(BooleanMembers, BoolArgConsumesTrueOrFalse) {
    // The explicit form consumes one of the two accepted values.
    const auto parsedTrue { Ogma::Parse<ExplicitBoolOptions>({{ "--flag", "true"  }}) };
    const auto parsedFalse{ Ogma::Parse<ExplicitBoolOptions>({{ "--flag", "false" }}) };

    ASSERT_TRUE(parsedTrue.has_value())  << parsedTrue.error();
    ASSERT_TRUE(parsedFalse.has_value()) << parsedFalse.error();
    EXPECT_TRUE(parsedTrue->first.flag);
    EXPECT_FALSE(parsedFalse->first.flag);
}

TEST(BooleanMembers, BoolArgRejectsOtherValues) {
    // Anything that is not "true"/"false" is rejected, naming the accepted values.
    const auto parsed{ Ogma::Parse<ExplicitBoolOptions>({{ "--flag", "maybe" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("true"), std::string::npos);
}

TEST(BooleanMembers, BoolArgRequiresAValue) {
    // The explicit form without a value is rejected, not read as the switch.
    const auto parsed{ Ogma::Parse<ExplicitBoolOptions>({{ "--flag" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("true"), std::string::npos);
}

#pragma endregion


#pragma region Optional Members

// ===================================================================================================================
// std::optional members are named-only (their ExpectedFlags omit ""), so they never compete for bare tokens and are
// never reported as missing. The inner type's parser handles the value.
// ===================================================================================================================

namespace {
    struct OptionalOptions {
        [[ =Ogma::Arg{ "name"  } ]] std::optional<std::string> name{};
        [[ =Ogma::Arg{ "count" } ]] std::optional<int>         count{};
    };
}

TEST(OptionalMembers, RemainUnsetWhenAbsent) {
    // Absent by default, filled by flag when present.
    const auto parsed{ Ogma::Parse<OptionalOptions>({{ "--name", "Ada" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_TRUE(parsed->first.name.has_value());
    EXPECT_EQ(*parsed->first.name, "Ada");
    EXPECT_FALSE(parsed->first.count.has_value());
}

TEST(OptionalMembers, UseTheirInnerParser) {
    // The value goes through the inner type's own parser.
    const auto parsed{ Ogma::Parse<OptionalOptions>({{ "--count", "7" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_TRUE(parsed->first.count.has_value());
    EXPECT_EQ(*parsed->first.count, 7);
}

#pragma endregion


#pragma region Repeated Occurrences

// ===================================================================================================================
// Testing the replace behavior when repeating a value.
// ===================================================================================================================

namespace {
    struct ScalarOptions {
        [[ =Ogma::Arg{ "value" } ]] int value{};
    };
}

TEST(RepeatedOccurrences, ScalarKeepsTheLastValue) {
    // Same flag twice, the parser sees the old value and replaces it.
    const auto parsed{ Ogma::Parse<ScalarOptions>({{ "--value", "1", "--value", "2" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.value, 2);
}

#pragma endregion


#pragma region Root Boundary

// ===================================================================================================================
// TypeParser::Parse never distinguishes root from nested, so a "-" at the true top is intercepted the same way and
// hands the remainder back to a caller that no longer exists. Ogma::Parse turns leftover input into an error, while a
// trailing "-" with nothing after it is harmless.
// ===================================================================================================================

TEST(RootBoundary, TrailingSeparatorIsHarmless) {
    // Nothing after the "-" means nothing is handed back.
    const auto parsed{ Ogma::Parse<ScalarOptions>({{ "--value", "1", "-" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.value, 1);
}

TEST(RootBoundary, LeftoverAfterSeparatorIsRejected) {
    // The root has no caller to hand the remainder to.
    const auto parsed{ Ogma::Parse<ScalarOptions>({{ "--value", "1", "-", "extra" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unexpected argument"), std::string::npos);
}

#pragma endregion