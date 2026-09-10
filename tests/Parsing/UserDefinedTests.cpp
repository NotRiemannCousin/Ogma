// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>

#include <gtest/gtest.h>

#include <array>
#include <expected>
#include <string>
#include <string_view>

#pragma region TrackId

// ===================================================================================================================
// Just making sure that it will work with third-party classes.
// ===================================================================================================================


namespace {
    struct TrackId {
        std::string value{};

        constexpr auto operator==(const TrackId&) const -> bool = default;
    };

    struct LowercaseOnly {};
    inline constexpr LowercaseOnly lowercaseOnly{};

    struct UserDefinedOptions {
        [[ =Ogma::Arg{ "track" }, =lowercaseOnly ]]
        TrackId track{};
    };

    struct PlainUserDefinedOptions {
        [[ =Ogma::Arg{ "track" } ]]
        TrackId track{};
    };
}


template<Ogma::Arg ArgDefinition>
struct Ogma::TypeParser<TrackId, ArgDefinition> {
    template<auto... Annotations>
    static Result<TrackId> Parse(const TrackId&, std::string_view, const ArgSpan args, const std::string_view) {
        if (args.empty())
            return std::unexpected{ "expected a track id" };

        std::string value{ args.front() };
        if (!value.starts_with("track:"))
            return std::unexpected{ "track id must start with 'track:'" };

        if constexpr (Utils::GetAnnotation<LowercaseOnly, Annotations...>()) {
            for (const char character : value) {
                if (character >= 'A' && character <= 'Z')
                    return std::unexpected{ "track id must be lowercase" };
            }
        }

        return std::make_pair(
            TrackId{ std::move(value) }, args.subspan(1));
    }

    template<auto...>
    static constexpr ArgSpan ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }

    template<auto... Annotations>
    static consteval ValueSpec Spec(const std::meta::info member) {
        return details_::DefaultSpec<TrackId, ArgDefinition, Annotations...>(member);
    }
};


TEST(UserDefinedTypes, CustomParserConsumesAndValidatesItsToken) {
    const auto parsed{ Ogma::Parse<UserDefinedOptions>({{ "--track", "track:abc123" }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.track.value, "track:abc123");
}

TEST(UserDefinedTypes, CustomAnnotationCanRejectAValue) {
    const auto parsed{ Ogma::Parse<UserDefinedOptions>({{ "--track", "track:ABC123" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("lowercase"), std::string::npos);
}

TEST(UserDefinedTypes, CustomParserReportsItsOwnSyntaxError) {
    const auto parsed{ Ogma::Parse<PlainUserDefinedOptions>({{ "--track", "video:abc123" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("track id must start"), std::string::npos);
}

#pragma endregion