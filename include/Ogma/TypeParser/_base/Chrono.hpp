#pragma once
#include <Ogma/TypeParser/_base.hpp>

#include <chrono>

namespace Ogma {

    //! @brief TypeParser specialization for @c std::chrono::duration<Rep, Period>.
    //!
    //! Parsing modes:
    //! @par Raw
    //! Expects a bare number in the duration's native unit, such as @c "500" for @c milliseconds{ 500 }.
    //! Floating-point @p Rep accepts decimal fractions; integral @p Rep requires a pure integer.
    //!
    //! @par Humantime (@ref DurationFmt)
    //! Accepts human-readable strings such as @c "2h 3m 1s", @c "1.5s" and , @c "500ms".
    //! This branch is independent of whether Rep is integral or floating-point, duration_cast performs the final
    //! conversion to T.
    //!
    //! This type is @ref positional-vs-named "positional-capable" in both modes.
    template<class Rep, class Period, Arg ArgDefinition>
    struct TypeParser<std::chrono::duration<Rep, Period>, ArgDefinition> {
        using Duration = std::chrono::duration<Rep, Period>;

        //! @brief Parses a duration in raw or human-readable mode selected by @p Annotations.
        template<auto... Annotations>
        static auto Parse(Duration curr, std::string_view flag, ArgSpan args, std::string_view scopePath = {}) -> Result<Duration>;

        //! @returns `{ "", fieldName }`.
        template<auto... Annotations>
        static constexpr ArgSpan ExpectedFlags();

        //! @brief Reports how the member shows up in the help page.
        //!
        //! Fill this together with @c Parse and @c ExpectedFlags, a specialisation that changes what it accepts per
        //! annotation (like the bool modes selected by @ref Ogma::BoolArg) should change this per annotation too.
        template<auto... Annotations>
        static consteval ValueSpec Spec(std::meta::info member);
    };

    //! @brief TypeParser specialization for @c std::chrono::time_point<Clock, Duration>.
    //!
    //! Parsing modes:
    //! @par ISO 8601
    //! Uses ISO 8601 by default.
    //!
    //! @par Unix seconds
    //! @ref unixSeconds selects seconds since the Unix epoch.
    //!
    //! @par Unix milliseconds
    //! @ref unixMilliseconds selects milliseconds since the Unix epoch. This type is
    //! @ref positional-vs-named "positional-capable".
    template<class Clock, class Duration, Arg ArgDefinition>
    struct TypeParser<std::chrono::time_point<Clock, Duration>, ArgDefinition> {
        using TimePoint = std::chrono::time_point<Clock, Duration>;

        //! @brief Parses a time point using the format selected by @p Annotations.
        template<auto... Annotations>
        static auto Parse(TimePoint curr, std::string_view flag, ArgSpan args, std::string_view scopePath = {}) -> Result<TimePoint>;

        //! @returns `{ "", fieldName }`.
        template<auto... Annotations>
        static constexpr ArgSpan ExpectedFlags();

        //! @brief Reports how the member shows up in the help page.
        //!
        //! Fill this together with @c Parse and @c ExpectedFlags, a specialisation that changes what it accepts per
        //! annotation (like the bool modes selected by @ref Ogma::BoolArg) should change this per annotation too.
        template<auto... Annotations>
        static consteval ValueSpec Spec(std::meta::info member);
    };

}

#include <Ogma/TypeParser/_base/Chrono.tpp>
