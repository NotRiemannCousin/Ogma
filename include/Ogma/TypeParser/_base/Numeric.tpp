#pragma once

#include <algorithm>
#include <array>
#include <format>
#include <limits>
#include <cmath>
#include <map>

namespace Ogma {

    namespace details_ {
        inline std::expected<double, std::string> ResolveUnits(
            std::string_view token, const std::flat_map<std::string_view, double>& units, bool strictlyDecreasing = true) {
            namespace vs = std::views;
            namespace rg = std::ranges;

            constexpr auto areCharInSameGroup{ [](const char a, const char b) {
                return (std::isalpha(b) && std::isalpha(a))
                    || (std::isdigit(b) && (a == '-' || a == '.' || std::isdigit(a)))
                    || (b == '.' && std::isdigit(a));
            } };
            constexpr auto isValid{ [](const auto view) {
                return rg::none_of(view, [](const char c){ return std::isspace(c); });
            } };
            constexpr auto toStringView{ [](const auto view) {
                return std::string_view{ view.begin(), view.size() };
            } };

            constexpr auto separateValuesAndUnits {
                vs::chunk_by(areCharInSameGroup)
                | vs::filter(isValid)
                | vs::transform(toStringView)
                | vs::chunk(2)
            };

            const auto order{ strictlyDecreasing
                ? std::strong_ordering::less
                : std::strong_ordering::greater };

            double totalAcc{};
            auto lastUnit{ units.end() };
            for (const auto&& pair : token | separateValuesAndUnits) {
                if (std::ranges::distance(pair) != 2)
                    return std::unexpected{ "invalid argument (possible missing)" };

                auto iter{ pair.begin() };
                const auto valueStr{ *iter++ };
                const auto unitStr{ *iter };

                auto multiplyConstant{ [&](const double value) mutable -> std::expected<double, std::string> {
                    namespace ch = std::chrono;

                    const auto unitMult{ units.find(unitStr) };

                    if (unitMult == units.end())
                        return std::unexpected{ std::format("unknown unit: {}", unitStr) };
                    if (lastUnit != units.end() && unitMult->second <=> lastUnit->second != order)
                        return std::unexpected{ std::format("units must be in order: {}", unitStr) };

                    lastUnit = unitMult;
                    return unitMult->second * value;
                } };

                if (const auto value{ Utils::GetNumber<double>(valueStr).and_then(multiplyConstant) })
                    totalAcc += *value;
                else return std::unexpected{ std::move(value).error() };
            }

            if (lastUnit == units.end())
                return std::unexpected{ "no units found" };
            return totalAcc;
        }


#pragma region Parsing Integer

        template<class T, IntegerFormat Format>
        struct ParseInteger;

        template<class T>
        struct ParseInteger<T, IntegerFormat::Hex> {
            std::expected<T, std::string> operator()(std::string_view token) const {
                if (!token.starts_with("0x") && !token.starts_with("0X"))
                    return std::unexpected{ "invalid number: expect a hexadecimal format (0x or 0X)" };
                return Utils::GetNumber<T>(token.substr(2), 16);
            }
        };
        template<class T>
        struct ParseInteger<T, IntegerFormat::Dec> {
            std::expected<T, std::string> operator()(std::string_view token) const {
                return Utils::GetNumber<T>(token, 10);
            }
        };
        template<class T>
        struct ParseInteger<T, IntegerFormat::Bin> {
            std::expected<T, std::string> operator()(std::string_view token) const {
                if (!token.starts_with("0b") && !token.starts_with("0B"))
                    return std::unexpected{ "invalid number: expect a binary format (0b or 0B)" };
                return Utils::GetNumber<T>(token.substr(2), 2);
            }
        };

        template<class T>
        struct ParseInteger<T, IntegerFormat::Si> {
            std::expected<T, std::string> operator()(const std::string_view token) const {
                constexpr auto convertToType{ [](const auto v) -> std::expected<T, std::string> {
                    const auto maxLimit{ std::numeric_limits<T>::max() };
                    const auto minLimit{ std::numeric_limits<T>::min() };

                    if (v > maxLimit || v < minLimit)
                        return std::unexpected{ "integer overflow" };

                    return static_cast<T>(v);
                } };

                static std::flat_map<std::string_view, double> prefixes{
                    { "u", 1  },
                    { "d", std::deca::num   }, { "h", std::hecto::num  }, { "k", std::kilo::num   },
                    { "K", std::kilo::num   }, { "M", std::mega::num   }, { "G", std::giga::num   },
                    { "T", std::tera::num   }, { "P", std::peta::num   }, { "E", std::exa::num    }
                };

                return ResolveUnits(token, prefixes).and_then(convertToType);
            }
        };

#pragma endregion

#pragma region Parsing Floating Point

        template<class T, FloatFormat Format>
        struct ParseFloat;

        template<class T>
        struct ParseFloat<T, FloatFormat::Fixed> {
            std::expected<T, std::string> operator()(const std::string_view token) const {
                return Utils::GetNumber<T>(token, std::chars_format::fixed);
            }
        };

        template<class T>
        struct ParseFloat<T, FloatFormat::Si> {
            std::expected<T, std::string> operator()(const std::string_view token) const {
                static std::flat_map<std::string_view, double> prefixes{
                    { "u", 1.0 },
                    { "d", std::deca::num }, { "h", std::hecto::num }, { "k", std::kilo::num },
                    { "K", std::kilo::num }, { "M", std::mega::num  }, { "G", std::giga::num },
                    { "T", std::tera::num }, { "P", std::peta::num  }, { "E", std::exa::num }
                };

                return ResolveUnits(token, prefixes).and_then([](const double v) -> std::expected<T, std::string> {
                    return static_cast<T>(v);
                });
            }
        };

#pragma endregion

    }

    template<std::integral T, Arg ArgDefinition>
        requires (!std::same_as<T, bool>)
    template<auto... Annotations>
    Result<T> TypeParser<T, ArgDefinition>::Parse(T, std::string_view, const ArgSpan args, std::string_view) {
        constexpr auto validateRange{ [](const auto value) -> Result<T> {
            if constexpr (constexpr auto range{ Utils::GetAnnotation<Range, Annotations...>() }) {
                if (value.first < range->min || value.first > range->max)
                    return std::unexpected{ std::format( "value {} is out of range [{}, {}]",
                        value.first, range->min, range->max) };
            }

            return value;
        } };

        constexpr auto inputFormat{ Utils::GetAnnotation<IntegerFmt, Annotations...>()
                .value_or(decNumber).format };


        if (args.empty())
            return std::unexpected{ "expected an integer" };

        return Utils::ConsumeOneAndForward<details_::ParseInteger<T, inputFormat>>(args)
                .and_then(validateRange);
    }


    template<std::floating_point T, Arg ArgDefinition>
        template<auto... Annotations>
        Result<T> TypeParser<T, ArgDefinition>::Parse(T, std::string_view, const ArgSpan args, std::string_view) {
        constexpr auto validateRangeAndSpecial{ [](const auto value) -> Result<T> {
            if constexpr (!Utils::GetAnnotation<AllowSpecialValues, Annotations...>()) {
                if (std::isnan(value.first) || std::isinf(value.first))
                    return std::unexpected{ "NaN and infinity are not allowed" };
            }

            if constexpr (constexpr auto range{ Utils::GetAnnotation<FRange, Annotations...>() }) {
                if (value.first < range->min || value.first > range->max)
                    return std::unexpected{ std::format("value {} is out of range [{}, {}]",
                        value.first, range->min, range->max) };
            }

            return value;
        } };

        constexpr auto inputFormat{ Utils::GetAnnotation<FloatFmt, Annotations...>()
                .value_or(fixedFloat).format };

        if (args.empty())
            return std::unexpected{ "expected a number argument" };

        return Utils::ConsumeOneAndForward<details_::ParseFloat<T, inputFormat>>(args)
                .and_then(validateRangeAndSpecial);
    }



    template<std::integral T, Arg ArgDefinition>
        requires (!std::same_as<T, bool>)
    template<auto...>
    constexpr ArgSpan TypeParser<T, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }

    template<std::floating_point T, Arg ArgDefinition>
    template<auto...>
    constexpr ArgSpan TypeParser<T, ArgDefinition>::ExpectedFlags() {
        static constexpr std::array<std::string_view, 2> list{ "", ArgDefinition.longName };
        return list;
    }


    template<std::integral T, Arg ArgDefinition>
        requires (!std::same_as<T, bool>)
    template<auto... Annotations>
    consteval ValueSpec TypeParser<T, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<T, ArgDefinition, Annotations...>(member) };

        constexpr auto format{ Utils::GetAnnotation<IntegerFmt, Annotations...>().value_or(decNumber).format };
        std::string mods{};

        if constexpr (format == IntegerFormat::Hex) mods += "[format: hex]";
        if constexpr (format == IntegerFormat::Bin) mods += "[format: binary]";
        if constexpr (format == IntegerFormat::Si)  mods += "[format: SI]";

        if constexpr (constexpr std::optional<Range> range{ Utils::GetAnnotation<Range, Annotations...>() }) {
            if (!mods.empty()) mods += ' ';
            std::array<char, 32> buffer{};

            mods += "[range: ";
            mods += std::string_view{ buffer.data(), std::to_chars(&*buffer.data(), &*buffer.rbegin(), range->min).ptr };
            mods += "..";
            mods += std::string_view{ buffer.data(), std::to_chars(&*buffer.data(), &*buffer.rbegin(), range->max).ptr };
            mods += ']';
        }

        spec.modifiers = mods;
        return spec;
    }

    template<std::floating_point T, Arg ArgDefinition>
    template<auto... Annotations>
    consteval ValueSpec TypeParser<T, ArgDefinition>::Spec(std::meta::info member) {
        auto spec{ details_::DefaultSpec<T, ArgDefinition, Annotations...>(member) };

        constexpr auto format{ Utils::GetAnnotation<FloatFmt, Annotations...>().value_or(fixedFloat).format };
        std::string mods{};

        if constexpr (format == FloatFormat::Si) mods += "[format: SI]";

        // if constexpr (constexpr auto range{ Utils::GetAnnotation<FRange, Annotations...>() }) {
        //     if (!mods.empty()) mods += ' ';
        //     mods += std::format("[range: {}..{}]", range->min, range->max); // no constexpr floating conversion yet
        //     mods = std::vector<std::string>{ mods, std::format("[range: {}..{}]", range->min, range->max) }
        //              | Utils::concatBySpace;
        // }

        if constexpr (Utils::GetAnnotation<AllowSpecialValues, Annotations...>())
            mods = std::vector<std::string>{ mods, "[allow infinity]" } | Utils::concatBySpace;

        spec.modifiers = mods;
        return spec;
    }
}
