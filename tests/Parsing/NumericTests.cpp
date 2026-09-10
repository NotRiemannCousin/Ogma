// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>

#include <gtest/gtest.h>


#pragma region Parsing In Struct

// ===================================================================================================================
// Parsing with different integer parsing behaviors.
// ===================================================================================================================


namespace {
    struct NumericOptions {
        [[ =Ogma::Arg{ "decimal" } ]]
        int decimal{};

        [[ =Ogma::Arg{ "hex"    }, =Ogma::hexNumber ]] unsigned hex{};
        [[ =Ogma::Arg{ "binary" }, =Ogma::binNumber ]] unsigned binary{};
        [[ =Ogma::Arg{ "si"     }, =Ogma::siNumber  ]] unsigned si{};

        [[ =Ogma::Arg{ "ratio" }, =Ogma::Range{ 0, 100 } ]]
        int ratio{};

        [[ =Ogma::Arg{ "scale" } ]]
        double scale{};
    };
}

TEST(Numeric, ParsesValuesInStruct1) {
    // The basic.
    const auto parsed{ Ogma::Parse<NumericOptions>({{
        "--decimal", "-17"   , "--hex"    , "0x2a",
        "--binary" , "0b1011", "--si"     , "1.5k",
        "--ratio"  , "75"    , "--scale"  , "1.25"
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.decimal, -17);
    EXPECT_EQ(parsed->first.hex    , 42u);
    EXPECT_EQ(parsed->first.binary , 11u);
    EXPECT_EQ(parsed->first.si     , 1500u);
    EXPECT_EQ(parsed->first.ratio  , 75);
    EXPECT_DOUBLE_EQ(parsed->first.scale, 1.25);
}

TEST(Numeric, RejectsMissingValue) {
    // Value not specified.
    const auto parsed{ Ogma::Parse<NumericOptions>({{ "--decimal" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("expected an integer"), std::string::npos);
}

#pragma endregion


#pragma region Template Parametrized Error Tests
// TYPED_TEST don't fit here

#pragma push_macro("PARSE_ERR_TEST")
#pragma push_macro("INTEGER_FMT_ERR_TEST")
#pragma push_macro("FLOAT_FMT_ERR_TEST")
#pragma push_macro("INTEGER_RANGE_ERR_TEST")
#pragma push_macro("FLOAT_RANGE_ERR_TEST")
#pragma push_macro("INTEGER_OVERFLOW_ERR_TEST")
#pragma push_macro("FLOAT_SPECIAL_ERR_TEST")

#undef PARSE_ERR_TEST
#undef INTEGER_FMT_ERR_TEST
#undef FLOAT_FMT_ERR_TEST
#undef INTEGER_RANGE_ERR_TEST
#undef FLOAT_RANGE_ERR_TEST
#undef INTEGER_OVERFLOW_ERR_TEST
#undef FLOAT_SPECIAL_ERR_TEST

// Test macros "gambiarra" for parsing with parenthesized isolation
#define PARSE_ERR_TEST(SUITE_NAME, T, NAME, ANNOTS, INPUT, EXP_ERR)                 \
TEST(SUITE_NAME, NAME) {                                                            \
    using IntParser = Ogma::TypeParser<T, Ogma::details_::internalArg>;             \
    const auto parsed{ IntParser::Parse<ANNOTS>({}, {}, {{ INPUT }}, {}) };         \
                                                                                    \
    ASSERT_FALSE(parsed.has_value());                                               \
    EXPECT_NE(parsed.error().find(EXP_ERR), std::string::npos) << parsed.error();   \
}

#define INTEGER_FMT_ERR_TEST(NAME, FMT, INPUT, EXP_ERR)        PARSE_ERR_TEST(IntegerFmt     , int   , NAME, (Ogma::IntegerFmt{ Ogma::IntegerFormat::FMT }), INPUT, EXP_ERR)
#define FLOAT_FMT_ERR_TEST(NAME, FMT, INPUT, EXP_ERR)          PARSE_ERR_TEST(FloatingFmt    , double, NAME, (Ogma::FloatFmt  { Ogma::FloatFormat::FMT }  ), INPUT, EXP_ERR)
#define INTEGER_RANGE_ERR_TEST(NAME, MIN, MAX, INPUT, EXP_ERR) PARSE_ERR_TEST(IntegerRange   , int   , NAME, (Ogma::Range     { MIN, MAX }                ), INPUT, EXP_ERR)
#define FLOAT_RANGE_ERR_TEST(NAME, MIN, MAX, INPUT, EXP_ERR)   PARSE_ERR_TEST(FloatingRange  , double, NAME, (Ogma::FRange    { MIN, MAX }                ), INPUT, EXP_ERR)
#define INTEGER_OVERFLOW_ERR_TEST(NAME, INPUT, EXP_ERR)        PARSE_ERR_TEST(IntegerOverflow, int   , NAME, (Ogma::decNumber                             ), INPUT, EXP_ERR)
#define FLOAT_SPECIAL_ERR_TEST(NAME, INPUT, EXP_ERR)           PARSE_ERR_TEST(FloatingSpecial, double, NAME, (Ogma::fixedFloat                            ), INPUT, EXP_ERR)


// Tests integer formatting in Decimal
INTEGER_FMT_ERR_TEST(Dec_NotANumber   , Dec, "not_a_number", "invalid number");
INTEGER_FMT_ERR_TEST(Dec_InvalidDec   , Dec, "1234ba"      , "invalid number");
INTEGER_FMT_ERR_TEST(Dec_NoDecBaseChar, Dec, "0d0000"      , "invalid number");
INTEGER_FMT_ERR_TEST(Dec_BinBase      , Dec, "0x1231"      , "invalid number");
INTEGER_FMT_ERR_TEST(Dec_HexBase      , Dec, "0b1231"      , "invalid number");

// Tests integer formatting in Binary
INTEGER_FMT_ERR_TEST(Bin_NotANumber   , Bin, "not_a_number", "expect a binary");
INTEGER_FMT_ERR_TEST(Bin_DecBase      , Bin, "123456"      , "expect a binary");
INTEGER_FMT_ERR_TEST(Bin_NoDecBaseChar, Bin, "0d0000"      , "expect a binary");
INTEGER_FMT_ERR_TEST(Bin_InvalidBin   , Bin, "0b0102"      , "invalid number" );
INTEGER_FMT_ERR_TEST(Bin_HexBase      , Bin, "0x0101"      , "expect a binary");

// Tests integer formatting in Hexadecimal
INTEGER_FMT_ERR_TEST(Hex_NotANumber   , Hex, "not_a_number", "expect a hexadecimal");
INTEGER_FMT_ERR_TEST(Hex_DecBase      , Hex, "123456"      , "expect a hexadecimal");
INTEGER_FMT_ERR_TEST(Hex_NoDecBaseChar, Hex, "0d0000"      , "expect a hexadecimal");
INTEGER_FMT_ERR_TEST(Hex_BinBase      , Hex, "0b0101"      , "expect a hexadecimal");
INTEGER_FMT_ERR_TEST(Hex_InvalidHex   , Hex, "0xefgh"      , "invalid number"      );

// Tests integer formatting with SI suffixes
INTEGER_FMT_ERR_TEST(Si_NotANumber    , Si, "not_a_number", "invalid number"        );
INTEGER_FMT_ERR_TEST(Si_Decimal       , Si, "123456"      , "invalid argument"      );
INTEGER_FMT_ERR_TEST(Si_HexAsUnits    , Si, "0x1023"      , "unknown unit"          );
INTEGER_FMT_ERR_TEST(Si_BinAsUnits    , Si, "0b1023"      , "unknown unit"          );
INTEGER_FMT_ERR_TEST(Si_MissingUnit   , Si, "10M 3"       , "invalid argument"      );
INTEGER_FMT_ERR_TEST(Si_OutOfOrder    , Si, "120k 3G"     , "units must be in order");
INTEGER_FMT_ERR_TEST(Si_Empty         , Si, ""            , "no units found"        );

// Tests range limits for integers
INTEGER_RANGE_ERR_TEST(BelowMin        ,  10,  20, "9"  , "out of range");
INTEGER_RANGE_ERR_TEST(AboveMax        ,  10,  20, "21" , "out of range");
INTEGER_RANGE_ERR_TEST(NegativeBelowMin, -10,  -5, "-11", "out of range");
INTEGER_RANGE_ERR_TEST(PositiveAboveMax, -10,  -5, "-4" , "out of range");

// Tests overflow limits for integers
INTEGER_OVERFLOW_ERR_TEST(PosOverflow,  "9999999999999999999", "invalid number");
INTEGER_OVERFLOW_ERR_TEST(NegOverflow, "-9999999999999999999", "invalid number");

// Tests float formatting in Fixed Point
FLOAT_FMT_ERR_TEST(Fixed_NotAFloat    , Fixed, "not_a_float", "invalid");
FLOAT_FMT_ERR_TEST(Fixed_GarbageSuffix, Fixed, "123.45abc"  , "invalid");
FLOAT_FMT_ERR_TEST(Fixed_DoubleDot    , Fixed, "1.2.3"      , "invalid");
FLOAT_FMT_ERR_TEST(Fixed_SciRejected  , Fixed, "1.5e4"      , "invalid");
FLOAT_FMT_ERR_TEST(Fixed_HexRejected  , Fixed, "0x1.fp3"    , "invalid");

// Tests float formatting with SI suffixes
FLOAT_FMT_ERR_TEST(Si_NotAFloat       , Si, "not_a_float" , "invalid"               );
FLOAT_FMT_ERR_TEST(Si_InvalidUnit     , Si, "1.5X"        , "unknown unit"          );
FLOAT_FMT_ERR_TEST(Si_OutOfOrder      , Si, "1.5k 2.0M"   , "units must be in order");
FLOAT_FMT_ERR_TEST(Si_Empty           , Si, ""            , "no units found"        );
FLOAT_FMT_ERR_TEST(Si_GarbageMid      , Si, "1.5k garbage", "invalid argument"      );

// Tests range limits for floats
FLOAT_RANGE_ERR_TEST(BelowMin          ,  0.5, 1.5, "0.49"       , "out of range");
FLOAT_RANGE_ERR_TEST(AboveMax          ,  0.5, 1.5, "1.51"       , "out of range");
FLOAT_RANGE_ERR_TEST(NegativeBelowMin  , -1.0, 0.0, "-1.01"      , "out of range");
FLOAT_RANGE_ERR_TEST(AboveMax_Precision,  1.0, 2.0, "2.000000001", "out of range");

// Tests rejection of special values for floats (NaN, Inf)
FLOAT_SPECIAL_ERR_TEST(NaN_Rejected        , "NaN" , "NaN and infinity are not allowed");
FLOAT_SPECIAL_ERR_TEST(Inf_Rejected        , "inf" , "NaN and infinity are not allowed");
FLOAT_SPECIAL_ERR_TEST(NegativeInf_Rejected, "-inf", "NaN and infinity are not allowed");
FLOAT_SPECIAL_ERR_TEST(NaNCapitalized      , "NAN" , "NaN and infinity are not allowed");

#pragma pop_macro("FLOAT_SPECIAL_ERR_TEST")
#pragma pop_macro("INTEGER_OVERFLOW_ERR_TEST")
#pragma pop_macro("FLOAT_RANGE_ERR_TEST")
#pragma pop_macro("INTEGER_RANGE_ERR_TEST")
#pragma pop_macro("FLOAT_FMT_ERR_TEST")
#pragma pop_macro("INTEGER_FMT_ERR_TEST")
#pragma pop_macro("PARSE_ERR_TEST")

#pragma endregion

#pragma region Template Parametrized Success Tests

#pragma push_macro("PARSE_INT_SUCCESS_TEST")
#pragma push_macro("PARSE_FLOAT_SUCCESS_TEST")
#pragma push_macro("INTEGER_FMT_SUCCESS_TEST")
#pragma push_macro("FLOAT_FMT_SUCCESS_TEST")
#pragma push_macro("INTEGER_SI_SUCCESS_TEST")
#pragma push_macro("FLOAT_SI_SUCCESS_TEST")
#pragma push_macro("FLOAT_SPECIAL_SUCCESS_TEST")

// Test macros for successful parsing (using EXPECT_DOUBLE_EQ for floats)
#define PARSE_INT_SUCCESS_TEST(SUITE_NAME, T, NAME, ANNOTS, INPUT, EXP_VAL)         \
TEST(SUITE_NAME, NAME) {                                                            \
    using IntParser = Ogma::TypeParser<T, Ogma::details_::internalArg>;             \
    const auto parsed{ IntParser::Parse<ANNOTS>({}, {}, {{ INPUT }}) };             \
                                                                                    \
    ASSERT_TRUE(parsed.has_value()) << parsed.error();                              \
    EXPECT_EQ(parsed->first, EXP_VAL);                                              \
}

#define PARSE_FLOAT_SUCCESS_TEST(SUITE_NAME, T, NAME, ANNOTS, INPUT, EXP_VAL)       \
TEST(SUITE_NAME, NAME) {                                                            \
    using FloatParser = Ogma::TypeParser<T, Ogma::details_::internalArg>;           \
    const auto parsed{ FloatParser::Parse<ANNOTS>({}, {}, {{ INPUT }}) };           \
                                                                                    \
    ASSERT_TRUE(parsed.has_value()) << parsed.error();                              \
    EXPECT_DOUBLE_EQ(parsed->first, EXP_VAL);                                       \
}

#define INTEGER_FMT_SUCCESS_TEST(NAME, FMT, INPUT, EXP_VAL)    PARSE_INT_SUCCESS_TEST  (IntegerFmtSuccess     , int      , NAME, (Ogma::IntegerFmt{ Ogma::IntegerFormat::FMT }), INPUT, EXP_VAL)
#define FLOAT_FMT_SUCCESS_TEST(NAME, FMT, INPUT, EXP_VAL)      PARSE_FLOAT_SUCCESS_TEST(FloatingFmtSuccess    , double   , NAME, (Ogma::FloatFmt  { Ogma::FloatFormat::FMT }  ), INPUT, EXP_VAL)
#define INTEGER_SI_SUCCESS_TEST(NAME, INPUT, EXP_VAL)          PARSE_INT_SUCCESS_TEST  (IntegerSiSuccess      , long long, NAME, (Ogma::IntegerFmt{ Ogma::IntegerFormat::Si } ), INPUT, EXP_VAL)
#define FLOAT_SI_SUCCESS_TEST(NAME, INPUT, EXP_VAL)            PARSE_FLOAT_SUCCESS_TEST(FloatingSiSuccess     , double   , NAME, (Ogma::FloatFmt  { Ogma::FloatFormat::Si }   ), INPUT, EXP_VAL)
#define FLOAT_SPECIAL_SUCCESS_TEST(NAME, INPUT, EXP_VAL)       PARSE_FLOAT_SUCCESS_TEST(FloatingSpecialSuccess, double   , NAME, (Ogma::allowSpecialValues                    ), INPUT, EXP_VAL)


// Test parsing success of binary, decimal and hexadecimal
INTEGER_FMT_SUCCESS_TEST(Dec_Zero    , Dec, "0"     ,    0);
INTEGER_FMT_SUCCESS_TEST(Dec_Positive, Dec, "123"   ,  123);
INTEGER_FMT_SUCCESS_TEST(Dec_Negative, Dec, "-456"  , -456);
INTEGER_FMT_SUCCESS_TEST(Hex_Lower   , Hex, "0x1a"  ,   26);
INTEGER_FMT_SUCCESS_TEST(Hex_Upper   , Hex, "0X2F"  ,   47);
INTEGER_FMT_SUCCESS_TEST(Hex_Zero    , Hex, "0x0"   ,    0);
INTEGER_FMT_SUCCESS_TEST(Bin_Lower   , Bin, "0b1010",   10);
INTEGER_FMT_SUCCESS_TEST(Bin_Upper   , Bin, "0B1111",   15);

// Test parsing success of fixed-point floats
FLOAT_FMT_SUCCESS_TEST(Fixed_Zero    , Fixed, "0.0"   ,    0.0);
FLOAT_FMT_SUCCESS_TEST(Fixed_Positive, Fixed, "123.45", 123.45);
FLOAT_FMT_SUCCESS_TEST(Fixed_Negative, Fixed, "-67.89", -67.89);
FLOAT_FMT_SUCCESS_TEST(Fixed_NoDec   , Fixed, "42"    ,   42.0);

// SI tests (integrals)
INTEGER_SI_SUCCESS_TEST(Si_Basic_K     , "5k"       ,       5000LL);
INTEGER_SI_SUCCESS_TEST(Si_Basic_M     , "2M"       ,    2000000LL);
INTEGER_SI_SUCCESS_TEST(Si_Negative    , "-10k"     ,     -10000LL);
INTEGER_SI_SUCCESS_TEST(Si_FloatToInt  , "1.5k"     ,       1500LL);
INTEGER_SI_SUCCESS_TEST(Si_FloatToInt_M, "1.25M"    ,    1250000LL);
INTEGER_SI_SUCCESS_TEST(Si_Combined    , "1M500k"   ,    1500000LL);
INTEGER_SI_SUCCESS_TEST(Si_Combined_Dec, "1.5M200k" ,    1700000LL);
INTEGER_SI_SUCCESS_TEST(Si_Unit_U      , "42u"      ,         42LL);
INTEGER_SI_SUCCESS_TEST(Si_Multiple    , "1G200M30k", 1200030000LL);

// SI tests (floating pointers)
FLOAT_SI_SUCCESS_TEST(Si_Basic_K     , "2.5k"      ,       2500.0);
FLOAT_SI_SUCCESS_TEST(Si_Basic_M     , "1.25M"     ,    1250000.0);
FLOAT_SI_SUCCESS_TEST(Si_Negative    , "-3.5k"     ,      -3500.0);
FLOAT_SI_SUCCESS_TEST(Si_Small_Prefix, "1.5d"      ,         15.0);
FLOAT_SI_SUCCESS_TEST(Si_Combined    , "1.5M200.5k",    1700500.0);
FLOAT_SI_SUCCESS_TEST(Si_Unit_U      , "42.5u"     ,         42.5);
FLOAT_SI_SUCCESS_TEST(Si_Multiple    , "1.1G2M"    , 1102000000.0);

// Special Values
FLOAT_SPECIAL_SUCCESS_TEST(PositiveInf1, "inf"      ,  std::numeric_limits<double>::infinity());
FLOAT_SPECIAL_SUCCESS_TEST(PositiveInf2, "infinity" ,  std::numeric_limits<double>::infinity());
FLOAT_SPECIAL_SUCCESS_TEST(PositiveInf3, "INFINITY" ,  std::numeric_limits<double>::infinity());
FLOAT_SPECIAL_SUCCESS_TEST(NegativeInf1, "-inf"     , -std::numeric_limits<double>::infinity());
FLOAT_SPECIAL_SUCCESS_TEST(NegativeInf2, "-infinity", -std::numeric_limits<double>::infinity());
FLOAT_SPECIAL_SUCCESS_TEST(NegativeInf3, "-INFINITY", -std::numeric_limits<double>::infinity());

// Special "special" test because NaN != NaN
TEST(FloatingSpecialSuccessTest, NaN_Lower) {
    using FloatParser = Ogma::TypeParser<double, Ogma::details_::internalArg>;

    for (const std::string_view arg : { "NaN", "Nan", "naN", "NaN", "NaN(my_bad)" }) {
        const auto parsed{ FloatParser::Parse<Ogma::allowSpecialValues>({}, {}, {{ arg }}) };

        ASSERT_TRUE(parsed.has_value()) << parsed.error();
        EXPECT_TRUE(std::isnan(parsed->first));
    }
}

#pragma pop_macro("FLOAT_SPECIAL_SUCCESS_TEST")
#pragma pop_macro("FLOAT_SI_SUCCESS_TEST")
#pragma pop_macro("INTEGER_SI_SUCCESS_TEST")
#pragma pop_macro("FLOAT_FMT_SUCCESS_TEST")
#pragma pop_macro("INTEGER_FMT_SUCCESS_TEST")
#pragma pop_macro("PARSE_FLOAT_SUCCESS_TEST")
#pragma pop_macro("PARSE_INT_SUCCESS_TEST")

#pragma endregion