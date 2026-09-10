#pragma once
#include <Ogma/TypeParser/_base.hpp>
#include <concepts>

namespace Ogma {

    //! @brief TypeParser specialization for all non-bool integral types.
    //!
    //! Reads the next token as an integer with @c std::from_chars. This type is
    //! @ref positional-vs-named "positional-capable".
    template<std::integral T, Arg ArgDefinition>
        requires (!std::same_as<T, bool>)
    struct TypeParser<T, ArgDefinition> {
        //! @brief Parse an integer from @p args.
        template<auto... Annotations>
        static Result<T> Parse(T curr, std::string_view, ArgSpan args, std::string_view scopePath = {});

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

    //! @brief TypeParser specialization for floating-point types.
    //!
    //! Reads the next token as a floating-point number with @c std::from_chars. This type is
    //! @ref positional-vs-named "positional-capable".
    template<std::floating_point T, Arg ArgDefinition>
    struct TypeParser<T, ArgDefinition> {
        //! @brief Parse a floating-point number from @p args.
        template<auto... Annotations>
        static Result<T> Parse(T curr, std::string_view, ArgSpan args, std::string_view scopePath = {});

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

#include <Ogma/TypeParser/_base/Numeric.tpp>
