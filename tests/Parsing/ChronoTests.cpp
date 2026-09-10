// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>
#include <gtest/gtest.h>

#include <chrono>

namespace ch = std::chrono;
using namespace std::chrono_literals;

#pragma region Durations

// ===================================================================================================================
// Duration -> Default format is a raw token integer read in the duration's native unit. With Ogma::durationFmt the
// value is a human-readable unit sequence ("1s 250ms") converted to the field's unit.
// ===================================================================================================================

namespace {
    struct RawHumanOptions {
        [[ =Ogma::Arg{ "raw" } ]] ch::milliseconds raw{};
        [[ =Ogma::Arg{ "human" }, =Ogma::durationFmt ]] ch::milliseconds human{};
    };

    struct HumanSecondsOptions {
        [[ =Ogma::Arg{ "human" }, =Ogma::durationFmt ]] ch::seconds human{};
    };
}

TEST(Chrono, ParsesRawAndHumanDurations) {
    // The basic.
    const auto parsed{ Ogma::Parse<RawHumanOptions>({{
        "--raw", "500", "--human", "1s 250ms"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.raw  , 500ms);
    EXPECT_EQ(parsed->first.human, 1250ms);
}

TEST(Chrono, ParsesCombinedHumanUnits) {
    // Units accumulate into the field's own unit.
    const auto parsed{ Ogma::Parse<HumanSecondsOptions>({{ "--human", "2h30m" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.human, 9000s);
}

TEST(Chrono, RejectsRawNonNumericValue) {
    // Raw durations are integers.
    const auto parsed{ Ogma::Parse<RawHumanOptions>({{ "--raw", "not_a_number" }}) };

    ASSERT_FALSE(parsed.has_value());
}

TEST(Chrono, RejectsHumanUnknownUnit) {
    // A human-readable duration with an unknown unit is rejected instead of being read as a plain number.
    const auto parsed{ Ogma::Parse<RawHumanOptions>({{ "--human", "3fortnights" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("unknown unit"), std::string::npos);
}

TEST(Chrono, RejectsHumanEmptyValue) {
    // A unit sequence is expected.
    const auto parsed{ Ogma::Parse<RawHumanOptions>({{ "--human", "" }}) };

    ASSERT_FALSE(parsed.has_value());
}

#pragma endregion


#pragma region Time Points

// ===================================================================================================================
// TimePoint -> Ogma::unixSeconds / Ogma::unixMilliseconds read bare integers since the epoch; the default format is
// ISO 8601 ("%FT%T%Ez"): date, time and a MANDATORY numeric UTC offset ("+00:00").
// ===================================================================================================================

namespace {
    struct UnixTimeOptions {
        [[ =Ogma::Arg{ "seconds"      }, =Ogma::unixSeconds      ]] ch::system_clock::time_point seconds{};
        [[ =Ogma::Arg{ "milliseconds" }, =Ogma::unixMilliseconds ]] ch::system_clock::time_point milliseconds{};
    };

    struct IsoOptions {
        [[ =Ogma::Arg{ "at" } ]] ch::system_clock::time_point at{};
    };
}

TEST(Chrono, ParsesIso8601WithUtcOffset) {
    // The offset is required and applied -- "-01:00" is one hour behind UTC, so the epoch value gains it.
    const auto utc{ Ogma::Parse<IsoOptions>({{ "--at", "1970-01-01T00:00:02+00:00" }}) };

    ASSERT_TRUE(utc.has_value()) << utc.error();
    EXPECT_EQ(utc->first.at.time_since_epoch(), 2s);

    const auto shifted{ Ogma::Parse<IsoOptions>({{ "--at", "1970-01-01T00:00:02-01:00" }}) };

    ASSERT_TRUE(shifted.has_value()) << shifted.error();
    EXPECT_EQ(shifted->first.at.time_since_epoch(), 3602s);
}

TEST(Chrono, RejectsIso8601WithoutUtcOffset) {
    // "%Ez" makes the offset mandatory, a raw timestamp is invalid.
    const auto parsed{ Ogma::Parse<IsoOptions>({{ "--at", "1970-01-01T00:00:02" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("invalid ISO 8601 time point"), std::string::npos);
}

TEST(Chrono, RejectsIso8601Garbage) {
    // Malformed timestamp.
    const auto parsed{ Ogma::Parse<IsoOptions>({{ "--at", "yesterday" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("invalid ISO 8601 time point"), std::string::npos);
}

TEST(Chrono, ParsesUnixSecondsAndMilliseconds) {
    // Both Unix timestamp formats.
    const auto parsed{ Ogma::Parse<UnixTimeOptions>({{
        "--seconds", "2", "--milliseconds", "2500"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.seconds.time_since_epoch()     , 2s);
    EXPECT_EQ(parsed->first.milliseconds.time_since_epoch(), 2500ms);
}

TEST(Chrono, RejectsUnixNonNumericValue) {
    // Unix formats are integers.
    const auto parsed{ Ogma::Parse<UnixTimeOptions>({{ "--seconds", "not_a_number" }}) };

    ASSERT_FALSE(parsed.has_value());
}

#pragma endregion