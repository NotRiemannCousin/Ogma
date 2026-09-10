// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>
#include <gtest/gtest.h>
#include <string_view>

#pragma region String

// ===================================================================================================================
// Testing string behaviour. It takes the next token as a string, but if the next token is "--" it will take the second
// token. Despite not working like POSIX, "--" isn't parsed as a flag by this library and it is used as an local
// "opt out" control token when the next token can be misunderstood as a flag (like in "-- --my-name").
// ===================================================================================================================

namespace {
    struct Options { [[ =Ogma::Arg{ "str" } ]] std::string str{}; };
}

TEST(StringTypes, ParsingByName) {
    // The basic.
    const auto parsed{ Ogma::Parse<Options>({{ "--str", "Green" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.str, "Green");
}

TEST(StringTypes, ParsingSpecialByName) {
    // Value misunderstood as an flag.
    const auto parsed{ Ogma::Parse<Options>({{ "--str", "--Green" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.str, "--Green");
}

TEST(StringTypes, FailsParsingExplicitSpecialByName) {
    // Using the controlToken.
    const auto parsed{ Ogma::Parse<Options>({{ "--str", "--", "--Green" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("--Green"), std::string::npos);
}
TEST(StringTypes, ParsingExplicitExplicit) {
    // Technically valid.
    const auto parsed{ Ogma::Parse<Options>({{ "--", "--" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.str, "--");
}




TEST(StringTypes, ParsingByPosition) {
    const auto parsed{ Ogma::Parse<Options>({{ "Green" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.str, "Green");
}

TEST(StringTypes, FailsParsingSpecialByPosition) {
    const auto parsed{ Ogma::Parse<Options>({{ "--Green" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("--Green"), std::string::npos);
}

TEST(StringTypes, ParsingExplicitSpecialByPosition) {
    const auto parsed{ Ogma::Parse<Options>({{ "--", "--Green" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.str, "--Green");
}

#pragma endregion