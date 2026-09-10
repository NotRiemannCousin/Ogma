// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>

#include <gtest/gtest.h>

#include <array>
#include <map>
#include <set>
#include <string_view>
#include <vector>

using StrMap = std::map<std::string, std::string>;

#pragma push_macro("EXPECT_RG_EQ")
#undef EXPECT_RG_EQ
#define EXPECT_RG_EQ(a, ...) EXPECT_TRUE(std::ranges::equal(a, (std::initializer_list{ __VA_ARGS__ })))

#pragma region Positional Type

// ===================================================================================================================
// Container<PositionalType> -> The container closes if anything starting with "-" is specified when reading a new
// element (including other flags). It will consume it only if it's a exactly match of "-".
// ===================================================================================================================

namespace {
    struct CollectionOptions {
        [[ =Ogma::Arg{ "values" } ]]
        std::vector<std::string> values{};

        [[ =Ogma::Arg{ "numbers" }, =Ogma::Fwd<Ogma::hexNumber>{} ]]
        std::vector<unsigned> numbers{};

        [[ =Ogma::Arg{ "unique" } ]]
        std::set<std::string> unique{};
    };

    struct RepeatedOptions {
        [[ =Ogma::Arg{ "value" } ]]
        std::vector<std::string> value{};
    };
}

TEST(PositionalTContainers, WithExplicitDelimiter) {
    // Closing the container explicitly with "-".
    const auto parsed{ Ogma::Parse<CollectionOptions>({{
        "--values" , "one"  , "two" , "-",
        "--numbers", "0x10" , "0x2a", "-",
        "--unique" , "alpha", "beta", "-",
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_RG_EQ(parsed->first.values , "one"  , "two" );
    EXPECT_RG_EQ(parsed->first.numbers, 16u    , 42u   );
    EXPECT_RG_EQ(parsed->first.unique , "alpha", "beta");
}

TEST(PositionalTContainers, WithoutExplicitDelimiter) {
    // Closing the container only using other flags.
    const auto parsed{ Ogma::Parse<CollectionOptions>({{
        "--values" , "one"  , "two" ,
        "--numbers", "0x10" , "0x2a",
        "--unique" , "alpha", "beta",
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_RG_EQ(parsed->first.values , "one"  , "two" );
    EXPECT_RG_EQ(parsed->first.numbers, 16u    , 42u   );
    EXPECT_RG_EQ(parsed->first.unique , "alpha", "beta");
}


TEST(PositionalTContainers, Fwd) {
    // Forwarding annotations.
    const auto parsed{ Ogma::Parse<CollectionOptions>({{ "--numbers", "0x1", "0x2", "0x3", "--values", "--unique" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_RG_EQ(parsed->first.numbers, 1u, 2u, 3u);
}

TEST(PositionalTContainers, FwdError) {
    // Forwarding annotations.
    const auto parsed{ Ogma::Parse<CollectionOptions>({{ "--numbers", "0xnope" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("invalid number"), std::string::npos);
}


TEST(PositionalTContainers, EmptyVector) {
    // Just specifying flags but doesn't add any element.
    const auto parsed{ Ogma::Parse<CollectionOptions>({{ "--values", "--numbers", "--unique" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_TRUE(parsed->first.values .empty());
    EXPECT_TRUE(parsed->first.numbers.empty());
    EXPECT_TRUE(parsed->first.unique .empty());
}

TEST(PositionalTContainers, RepeatedOptionAppendValues) {
    // Appending values.
    const auto parsed{ Ogma::Parse<RepeatedOptions>({{ "--value", "first", "--value", "second", "third" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_RG_EQ(parsed->first.value, "first", "second", "third");
}


TEST(PositionalTContainers, ParsesWithScapingElements) {
    // "--" is also used to space values by other types.
    const auto parsed{ Ogma::Parse<RepeatedOptions>({{ "--value", "--", "--first", "--", "--second", "--", "--third" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_RG_EQ(parsed->first.value, "--first", "--second", "--third");
}

#pragma endregion


#pragma region NamedOnly Type

// ===================================================================================================================
// Container<NamedOnlyType> -> The container only closes if "-" is specified when reading a new element.
// ===================================================================================================================

namespace {
    struct MountOptions {
        [[ =Ogma::Arg{ "source" } ]] std::string source{};
        [[ =Ogma::Arg{ "target" } ]] std::string target{};
    };

    struct MountPlan {
        [[ =Ogma::Arg{ "mounts" } ]] std::vector<MountOptions> mounts{};
    };
}

TEST(NamedOnlyTContainers, FailsWithoutExplicitStart) {
    // Needs "--" to starts a new element.
    const auto parsed{ Ogma::Parse<MountPlan>({{
        "--mounts", "/dev/sda1", "/mnt/a", "-",
        "--mounts", "/dev/sda2", "/mnt/b", "-",
        "-"
    }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("\"--\" is required to open"), std::string::npos);
}

TEST(NamedOnlyTContainers, ParsesRepeatedElements) {
    // Starts a new element with "--". "-" closes not only each MountPlan element but also the container itself.
    const auto parsed{ Ogma::Parse<MountPlan>({{
        "--mounts", "--", "--source", "/dev/sda1", "--target", "/mnt/a", "-",
        "--mounts", "--", "--source", "/dev/sda2", "--target", "/mnt/b", "-",
        "-"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_EQ(parsed->first.mounts.size(), 2u);
    EXPECT_EQ(parsed->first.mounts[0].source, "/dev/sda1");
    EXPECT_EQ(parsed->first.mounts[0].target, "/mnt/a");
    EXPECT_EQ(parsed->first.mounts[1].source, "/dev/sda2");
    EXPECT_EQ(parsed->first.mounts[1].target, "/mnt/b");
}


TEST(NamedOnlyTContainers, RepeatedElementsToleratesMissingFinalTerminator) {
    // For both, the named-only element and the container the closing "-" may be omitted when nothing else follows.
    const auto parsed{ Ogma::Parse<MountPlan>({{
        "--mounts", "--", "--source", "/dev/sda1", "--target", "/mnt/a"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_EQ(parsed->first.mounts.size(), 1u);
}

TEST(NamedOnlyTContainers, OmittingSeparatorBetweenElementsExtendsCurrentElement) {
    // ParseStruct takes everything until hit a "-", so "--source" and "--target" will just be replace the current
    // value instead of creating a new one.
    const auto parsed{ Ogma::Parse<MountPlan>({{
        "--mounts", "--", // just one "--" opened, one single value
        "--source", "/dev/sda1", "--target", "/mnt/a",
        "--source", "/dev/sda2", "--target", "/mnt/b", "-"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    ASSERT_EQ(parsed->first.mounts.size(), 1u);
    EXPECT_EQ(parsed->first.mounts[0].source, "/dev/sda2");
    EXPECT_EQ(parsed->first.mounts[0].target, "/mnt/b");
}

#pragma endregion


#pragma region MapType

// ===================================================================================================================
// map<string, string>: "--flag key=value" repeated, self-delimiting exactly
// like a scalar vector/set.
// ===================================================================================================================

namespace {
    struct EnvOptions {
        [[ =Ogma::Arg{ "env" } ]]
        std::map<std::string, std::string> env{};
    };
}

TEST(MapContainer, ParsesKeyValuePairs) {
    // Basic parsing.
    const auto parsed{ Ogma::Parse<EnvOptions>({{ "--env", "HOST=localhost", "PORT=8080" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.env, (StrMap{ { "HOST", "localhost" }, { "PORT", "8080" } }));
}

TEST(MapContainer, RepeatedKeyOverwritesThePreviousValue) {
    // Same key -> will replace.
    const auto parsed{ Ogma::Parse<EnvOptions>({{ "--env", "HOST=localhost", "HOST=example.com" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.env, (StrMap{ { "HOST", "example.com" } }));
}

TEST(MapContainer, RepeatedMapOptionAppendPairs) {
    // appending values.
    const auto parsed{ Ogma::Parse<EnvOptions>({{ "--env", "HOST=localhost", "--env", "PORT=8080" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.env, (StrMap{ { "HOST", "localhost" }, { "PORT", "8080" } }));
}

TEST(MapContainer, RejectsKeyValuePairWithoutSeparator) {
    // Missing '='.
    const auto parsed{ Ogma::Parse<EnvOptions>({{ "--env", "HOST" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("expected a \"key=value\" pair"), std::string::npos);
}

TEST(MapContainer, AcceptsKeyValuePairWithEmptyValue) {
    // "This option exists and it value is empty".
    const auto parsed{ Ogma::Parse<EnvOptions>({{ "--env", "HOST=" }}) };

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->first.env, (StrMap{ { "HOST", "" } }));
}
TEST(MapContainer, RejectsKeyValuePairWithEmptyKey) {
    // Well, at least the key needs to be presented.
    const auto parsed{ Ogma::Parse<EnvOptions>({{ "--env", "=localhost" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("key shouldn't be empty"), std::string::npos);
}

#pragma endregion


#pragma pop_macro("EXPECT_RG_EQ")