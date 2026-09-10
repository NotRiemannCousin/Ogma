#pragma once

//! @file Ogma/Annotations/Chrono.hpp
//! @brief Annotations for chrono duration and time point parsers.

namespace Ogma {

    //! @brief Enables human readable duration parsing.
    //!
    //! @par Help
    //! Appended as "[format: human time]".
    struct DurationFmt {};

    //! @brief The annotation value used to enable human readable durations.
    inline constexpr DurationFmt durationFmt{};

    //! @brief Selects the wire format accepted for a time point.
    enum class TimePointFormat { Iso8601, UnixSeconds, UnixMilliseconds };

    //! @brief Controls parsing of chrono time point values.
    //!
    //! @par Help
    //! The selected format is appended as a modifier: "[format: unix seconds]", "[format: unix milliseconds]" or
    //! "[format: ISO 8601]" (the default).
    struct TimePointFmt {
        TimePointFormat format{ TimePointFormat::Iso8601 };
        constexpr auto operator==(const TimePointFmt&) const -> bool = default;
    };

    //! @brief Parses ISO 8601 time points.
    inline constexpr TimePointFmt iso8601{};

    //! @brief Parses Unix timestamps measured in seconds.
    inline constexpr TimePointFmt unixSeconds{ TimePointFormat::UnixSeconds };

    //! @brief Parses Unix timestamps measured in milliseconds.
    inline constexpr TimePointFmt unixMilliseconds{ TimePointFormat::UnixMilliseconds };

}
