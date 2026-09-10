// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>

#include <gtest/gtest.h>

#include <string_view>
#include <vector>

#pragma region Basic Constraints

// ===================================================================================================================
// EnumType -> Parses enumerators by mapping their names to string values.
// Supports scoped and unscoped enums, case insensitivity (type or field level), and multiple aliases.
// ===================================================================================================================

namespace {
    enum class PlainColor { Red, Green, Blue };

    enum UnscopedColor { Cyan, Magenta, Yellow };

    enum class [[ =Ogma::caseInsensitive ]] Format {
        Mp3 [[ =Ogma::Alias{ "audio" } ]],
        Flac,
        Opus
    };

    enum class MultiAlias {
        Start [[ =Ogma::Alias{ "begin" }, =Ogma::Alias{ "run" }, =Ogma::Alias{ "execute" } ]],
        Stop  [[ =Ogma::Alias{ "halt"  } ]]
    };

    struct EnumOptions {
        [[ =Ogma::Arg{ "plain"    } ]] std::optional<PlainColor>    plain    { PlainColor::Red  };
        [[ =Ogma::Arg{ "unscoped" } ]] std::optional<UnscopedColor> unscoped { Cyan             };
        [[ =Ogma::Arg{ "format"   } ]] std::optional<Format>        format   { Format::Opus     };
        [[ =Ogma::Arg{ "multi"    } ]] std::optional<MultiAlias>    multi    { MultiAlias::Stop };

        [[ =Ogma::Arg{ "field-insensitive" }, =Ogma::Fwd<Ogma::caseInsensitive>() ]]
        std::optional<PlainColor> fieldInsensitive{ PlainColor::Red };
    };
}

TEST(Enum, ParsesExactMatchesScopedAndUnscoped) {
    // Basic parsing for both scoped and unscoped enums matching exactly the identifier name.
    const auto parsed{ Ogma::Parse<EnumOptions>({{
        "--plain"   , "Green",
        "--unscoped", "Magenta"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.plain   , PlainColor::Green);
    EXPECT_EQ(parsed->first.unscoped, Magenta);
}

TEST(Enum, FailsWhenCaseSensitiveMismatches) {
    // Enums are case-sensitive by default, so "green" instead of "Green" should fail.
    const auto parsed{ Ogma::Parse<EnumOptions>({{ "--plain", "green" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("invalid value 'green'"), std::string::npos);
}

TEST(Enum, ParsesCaseInsensitiveOnType) {
    // The Format enum has CaseInsensitive at the type level, so mixed case should be accepted.
    const auto parsed{ Ogma::Parse<EnumOptions>({{ "--format", "fLaC" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.format, Format::Flac);
}

TEST(Enum, ParsesCaseInsensitiveOnField) {
    // If the enum *field* is case-insensitive, aliases must also be evaluated case-insensitively.
    const auto parsed1{ Ogma::Parse<EnumOptions>({{ "--field-insensitive", "bLuE" }}) };
    const auto parsed2{ Ogma::Parse<EnumOptions>({{ "--field-insensitive", "BlUe" }}) };
    ASSERT_TRUE(parsed1.has_value()) << parsed1.error();
    ASSERT_TRUE(parsed2.has_value()) << parsed2.error();
    EXPECT_EQ(parsed1->first.fieldInsensitive, PlainColor::Blue);
    EXPECT_EQ(parsed2->first.fieldInsensitive, PlainColor::Blue);
}

TEST(Enum, ParsesAliasesAndCaseInsensitiveAliases) {
    // If the enum *type* is case-insensitive, aliases must also be evaluated case-insensitively.
    const auto parsed1{ Ogma::Parse<EnumOptions>({{ "--format", "audio" }}) };
    const auto parsed2{ Ogma::Parse<EnumOptions>({{ "--format", "AuDiO" }}) };
    ASSERT_TRUE(parsed1.has_value()) << parsed1.error();
    ASSERT_TRUE(parsed2.has_value()) << parsed2.error();
    EXPECT_EQ(parsed1->first.format, Format::Mp3);
    EXPECT_EQ(parsed2->first.format, Format::Mp3);
}

TEST(Enum, ParsesMultipleAliases) {
    // Multiple aliases on the same enumerator are all valid options.
    const auto parsed1{ Ogma::Parse<EnumOptions>({{ "--multi", "begin"   }}) };
    const auto parsed2{ Ogma::Parse<EnumOptions>({{ "--multi", "run"     }}) };
    const auto parsed3{ Ogma::Parse<EnumOptions>({{ "--multi", "execute" }}) };

    ASSERT_TRUE(parsed1.has_value()) << parsed1.error();
    ASSERT_TRUE(parsed2.has_value()) << parsed2.error();
    ASSERT_TRUE(parsed3.has_value()) << parsed3.error();

    EXPECT_EQ(parsed1->first.multi, MultiAlias::Start);
    EXPECT_EQ(parsed2->first.multi, MultiAlias::Start);
    EXPECT_EQ(parsed3->first.multi, MultiAlias::Start);
}

TEST(Enum, FailsWithCorrectErrorMessageForInvalidValue) {
    // Invalid values should list all available choices.
    const auto parsed{ Ogma::Parse<EnumOptions>({{ "--plain", "Yellow" }}) };

    ASSERT_FALSE(parsed.has_value());

    const auto& err{ parsed.error() };
    EXPECT_NE(err.find("invalid value 'Yellow'"), std::string::npos);
    EXPECT_NE(err.find("expected one of:"), std::string::npos);

    // FlatMap keys are ordered, so we check for individual words instead of exact string match
    EXPECT_NE(err.find("Red")  , std::string::npos);
    EXPECT_NE(err.find("Green"), std::string::npos);
    EXPECT_NE(err.find("Blue") , std::string::npos);
}

TEST(Enum, InternalParserFailsOnEmptyArgs) {
    // If no arguments are provided to the TypeParser it should fail gracefully, also listing options.
    constexpr Ogma::Arg argDef{ "plain" };
    const auto result{ Ogma::TypeParser<PlainColor, argDef>::Parse({}, "", {}) };

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("expected one of:"), std::string::npos);
}

#pragma endregion