#pragma once

//! @file Ogma/Annotations/Numeric.hpp
//! @brief Annotations for numeric value formats.

namespace Ogma {

#pragma region Integral Values

    //! @brief Selects the accepted syntax for integral command line values.
    enum class IntegerFormat { Dec, Hex, Bin, Si };

    //! @brief Controls integral value parsing.
    //!
    //! @par Help
    //! The selected format is appended to the member's description as a modifier: "[format: hex]" for @ref hexNumber,
    //! "[format: binary]" for @ref binNumber, "[format: SI]" for @ref siNumber. Decimal (the default) renders no
    //! modifier.
    struct IntegerFmt {
        IntegerFormat format{ IntegerFormat::Dec };
        constexpr auto operator==(const IntegerFmt&) const -> bool = default;
    };

    //! @brief Accepts decimal values.
    inline constexpr IntegerFmt decNumber{ IntegerFormat::Dec };

    //! @brief Accepts hexadecimal values with a 0x prefix.
    inline constexpr IntegerFmt hexNumber{ IntegerFormat::Hex };

    //! @brief Accepts binary values with a 0b prefix.
    inline constexpr IntegerFmt binNumber{ IntegerFormat::Bin };

    //! @brief Accepts SI suffixes such as k, M, G, T, P, E, Z, and Y.
    inline constexpr IntegerFmt siNumber{ IntegerFormat::Si };



    //! @brief Constrains an integral argument to an inclusive range.
    //!
    //! @par Help
    //! Appended as "[range: min..max]", e.g. "[range: 0..100]".
    struct Range {
        long long min{};
        long long max{};
    };

#pragma endregion


#pragma region Floating-Point Values

    //! @brief Selects the accepted syntax for floating-point command line values.
    enum class FloatFormat { Fixed, Si };

    //! @brief Controls floating-point value parsing.
    //!
    //! @par Help
    //! "[format: SI]" when @ref siFloat is selected, fixed-point (the default) renders no modifier.
    struct FloatFmt {
        FloatFormat format{ FloatFormat::Fixed };
        constexpr auto operator==(const FloatFmt&) const -> bool = default;
    };


    //! @brief Accepts strict fixed-point values.
    inline constexpr FloatFmt fixedFloat{ FloatFormat::Fixed };

    //! @brief Accepts SI suffixes (k, M, G, etc).
    inline constexpr FloatFmt siFloat{ FloatFormat::Si };


    //! @brief Allows special floating-point values.
    //!
    //! @par Help
    //! Appended as "[allow infinity]".
    struct AllowSpecialValues {};

    //! @brief The annotation value used to enable NaN, +inf, and -inf.
    inline constexpr AllowSpecialValues allowSpecialValues{};



    //! @brief Constrains a floating point argument to an inclusive range.
    //!
    //! @par Help
    //! Appended as "[range: min..max]" (disabled, no constexpr floating-point formatting yet).
    struct FRange {
        long double min{};
        long double max{};
    };

    //! @brief Constrains an argument to the interval [0, 1].
    inline constexpr FRange ratio{ 0, 1 };


#pragma endregion

}