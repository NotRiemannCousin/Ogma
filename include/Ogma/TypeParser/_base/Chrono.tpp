#pragma once

#include <array>
#include <format>
#include <optional>
#include <flat_map>
#include <spanstream>

#include <Ogma/TypeParser/_base/Numeric.hpp>

#pragma push_macro("UNIT_RATIO")
#define NS_RATIO(UNIT) std::ratio_divide<std::chrono::UNIT::period, std::nano>::num

namespace Ogma {

    namespace details_ {

        template<class T>
        std::expected<T, std::string> ParseHumantimeNanos(const std::string_view token) {
            constexpr auto convertToDuration{ [](const auto totalNanos) {
                using SourceDuration = std::chrono::duration<double, std::nano>;

                return std::chrono::duration_cast<T>(SourceDuration{ totalNanos });
            } };

            static std::flat_map<std::string_view, double> units{
                { "ns"     , NS_RATIO(nanoseconds)  }, { "us"     , NS_RATIO(microseconds) },
                { "\u00b5s", NS_RATIO(microseconds) }, { "ms"     , NS_RATIO(milliseconds) },
                { "s"      , NS_RATIO(seconds)      }, { "m"      , NS_RATIO(minutes)      },
                { "h"      , NS_RATIO(hours)        }, { "d"      , NS_RATIO(days)         },
                { "w"      , NS_RATIO(weeks)        }, { "M"      , NS_RATIO(months)       },
                { "y"      , NS_RATIO(years)        }, { "Y"      , NS_RATIO(years)        },
            };

            return ResolveUnits(token, units).transform(convertToDuration);
        }
    }

    template<class Rep, class Period, Arg ArgDefinition>
    template<auto... Annotations>
    auto TypeParser<std::chrono::duration<Rep, Period>, ArgDefinition>::Parse(
        Duration, std::string_view, ArgSpan args, std::string_view) -> Result<Duration> {
        using T = Duration;
        if (args.empty()) return std::unexpected{ "expected a duration" };

        if constexpr (Utils::GetAnnotation<DurationFmt, Annotations...>())
            return Utils::ConsumeOneAndForward<details_::ParseHumantimeNanos<Duration>>(args);
        else {
            using Dur = std::conditional_t<std::chrono::treat_as_floating_point_v<Rep>, Rep, long long>;
            if (const auto res{ Utils::GetNumber<Dur>(args[0]) })
                return std::make_pair(T{ static_cast<Rep>(*res) }, args.subspan(1));
            return std::unexpected{ std::format("invalid duration: '{}'", args[0]) };
        }
    }


    namespace details_ {
        // - "Mom, can we get pattern matching?"
        // - "Sweet, we already have pattern matching at home"
        // The pattern matching at home:
        template<class T, TimePointFormat Format>
        struct ParseTimePoint {
            std::expected<T, std::string> operator()(std::string_view token) const {
                namespace ch = std::chrono;
                using Tpf = TimePointFormat;
                static_assert(Format == Tpf::UnixSeconds || Format == Tpf::UnixMilliseconds, "Unspecialized time format");

                using TimeUnit = std::conditional_t<Format == Tpf::UnixSeconds, ch::seconds, ch::milliseconds>;

                if (const auto count{ Utils::GetNumber<typename T::duration::rep>(token) }) {
                    const auto duration{ ch::duration_cast<typename T::duration>(TimeUnit{ *count }) };
                    return T{ duration };
                }
                return std::unexpected{ std::format("invalid Unix time point: '{}'", token) };
            }
        };

        template<class T>
        struct ParseTimePoint<T, TimePointFormat::Iso8601> {
            std::expected<T, std::string> operator()(std::string_view token) const {
                std::ispanstream input{ std::span{ token.data(), token.size() } };
                std::chrono::sys_time<typename T::duration> parsed{};

                input >> std::chrono::parse("%FT%T%Ez", parsed);
                if (input.fail())
                    return std::unexpected{ std::format("invalid ISO 8601 time point: '{}'", token) };

                return T{ parsed.time_since_epoch() };
            }
        };
    }

    template<class Clock, class Duration, Arg ArgDefinition>
    template<auto... Annotations>
    auto TypeParser<std::chrono::time_point<Clock, Duration>, ArgDefinition>::Parse(
        TimePoint, std::string_view, ArgSpan args, std::string_view) -> Result<TimePoint> {
        using T = TimePoint;
        if (args.empty()) return std::unexpected{ "expected a time point" };

        constexpr auto inputFormat{ Utils::GetAnnotation<TimePointFmt, Annotations...>()
                .value_or(iso8601).format };

        return Utils::ConsumeOneAndForward<details_::ParseTimePoint<T, inputFormat>>(args);
    }

    template<class Rep, class Period, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::chrono::duration<Rep, Period>, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }

    template<class Clock, class Duration, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<std::chrono::time_point<Clock, Duration>, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }



    template<class Rep, class Period, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::chrono::duration<Rep, Period>, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<std::chrono::duration<Rep, Period>, ArgDefinition, Annotations...>(member) };
        if constexpr (Utils::GetAnnotation<DurationFmt, Annotations...>())
            spec.modifiers = "[format: human time]";
        return spec;
    }

    template<class Clock, class Duration, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<std::chrono::time_point<Clock, Duration>, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<std::chrono::time_point<Clock, Duration>, ArgDefinition, Annotations...>(member) };
        constexpr auto format{ Utils::GetAnnotation<TimePointFmt, Annotations...>().value_or(iso8601).format };

        if constexpr (format == TimePointFormat::UnixSeconds     ) spec.modifiers = "[format: unix seconds]";
        if constexpr (format == TimePointFormat::UnixMilliseconds) spec.modifiers = "[format: unix milliseconds]";
        if constexpr (format == TimePointFormat::Iso8601         ) spec.modifiers = "[format: ISO 8601]";

        return spec;
    }

}


#pragma pop_macro("UNIT_RATIO")
