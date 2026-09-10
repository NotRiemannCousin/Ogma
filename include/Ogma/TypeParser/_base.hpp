#pragma once

//! @file Ogma/TypeParser/_base.hpp
//! @brief Core infrastructure: TypeParser<T> and details_:: traits and helpers.
//!
//! This file contains only declarations and lightweight inline/consteval helpers. All non-trivial template bodies live
//! in @c _base.tpp (the default struct-walking engine and its per-type plumbing) and in the per-type specialization
//! files under @ref include/Ogma/TypeParser .

#include <Ogma/_base.hpp>
#include <Ogma/Utils.hpp>
#include <Ogma/TypeParser/Help.hpp>

#include <algorithm>
#include <flat_map>
#include <meta>
#include <span>
#include <string_view>
#include <vector>

namespace Ogma {
    //! @brief Specialise this to teach Ogma how to parse @c T from command-line tokens. It's very similar to
    //! @c std::formatter, but for reading (see @ref type-parser "TypeParser structure").
    //!
    //! This is the default implementation used when no specialization is provided.
    //!
    //! @section General form
    //! Every specialisation needs to provide:
    //! @code
    //!   template<auto... Annotations>
    //!   static Result<T> Parse(T curr, std::string_view flag, ArgSpan args);
    //!   template<auto... Annotations>
    //!   static constexpr ArgSpan ExpectedFlags();
    //!   template<auto... Annotations>
    //!   static consteval ValueSpec Spec(std::meta::info member);
    //! @endcode
    //!
    //! @tparam T is the type that will be parsed.
    //! @tparam ArgDefinition is the annotation that describes the given option
    //! (like in `[[ =Ogma::Arg{ "numbers" } ]]`).
    //!
    //! They are the mandatory arguments that needs to be presented, but every function has a @c Annotations template
    //! pack that can carry the remaining annotations of the option, so it's possible to specialize their behaviour
    //! based in other annotations (like `[[ =Ogma::Arg{ "config" }, =Ogma::mustExist, =Ogma::fileOnly ]]`).
    //!
    //! @par ExpectedFlags
    //!
    //! @c ExpectedFlags is a function that returns every possible flag than can match with this option (without the
    //! "--" prefix). When you compile a program with `"g++ main.cpp -o main.exe -freflection -std=c++26"`, `"-o"`,
    //! `"-freflection"` and `"-std=c++26"` are flags used to select some behaviour.
    //!
    //! In Ogma every option can be specified by multiple different flags, but normally you will need just
    //! `{ "", ArgDefinition.longName }` (see @ref positional-vs-named "Positional vs Named Arguments" to know how
    //! the empty flag works).
    //!
    //! @par Parse
    //!
    //! @c Parse is responsible to the actual parsing of the arguments. It has the @c flag used to match with this
    //! option (if applicable), just like `"--cool-flag"` or `"--use-match"`.
    //!
    //! The @c curr field is the old living object passed by value to @c Parse (you can move it to @c Parse). It exists
    //! because a argument can pass through @c Parse multiple times. Things like the @c std::chrono classes or numerics
    //! types just ignores the old value, but containers can append values when called multiple times (they can also
    //! discard the old values. For consistency, if you need to specify this relation of conditional append/override
    //! you can use @ref Ogma::Overwrite or @ref Ogma::Append).
    //!
    //! Notably, in the example from the last paragraph `-std=c++26` sets not only the flag but also the value it takes,
    //! so @c Parse receives both the @c flag matched and the remaining @c args to set the value.
    //!
    //! @par Spec
    //!
    //! @c Spec is the third contract point. It never affects parsing, it exists only so the help page can show what the
    //! parser actually accepts. The @c member parameter carries the reflection of the data member being documented,
    //! this way a specialisation can derive semantic metavars with things like `std::meta::identifier_of`, or tell a
    //! declared default apart from a value-initialised field with `std::meta::has_default_member_initializer`. Filling
    //! it is the one place where a specialisation is allowed to be lazy, the fallback composes every field from
    //! @c ArgDefinition and the member identifier.
    //!
    //! The full extension contract is documented in @ref type-parser "TypeParser structure".
    //!
    //! @section Default behaviour
    //!
    //! This is the fallback behaviour of @c TypeParser. For any @p T with no specialisation of its own, it will act as
    //! a struct-walking engine towards all Ogma eligible members (with @ref Ogma::Arg "Arg" or
    //! @ref Ogma::VariantArg "VariantArg"), handling positional and named flag.
    template<class T, Arg ArgDefinition>
    struct TypeParser {
        //! @brief Primary fallback to parse values.
        //!
        //! This is the recursive struct-walking engine, no special annotation available.
        template<auto... Annotations>
        static Result<T> Parse(T curr, std::string_view flag, ArgSpan args, std::string_view scopePath = {});

        //! @brief Primary fallback flags.
        //!
        //! Returns `{ ArgDefinition.longName }` because it's a @ref positional-vs-named "named only argument".
        template<auto... Annotations>
        static constexpr ArgSpan ExpectedFlags();

        //! @brief Primary fallback Spec.
        //!
        //! Delegates to @c details_::DefaultSpec, so a member shows the @c ArgDefinition flag, a comma separated list
        //! of every flag it answers to, and a metavariable derived from its own identifier. Specialisations override
        //! it when the value has a shape the identifier cannot convey, an enum lists its choices and a map shows
        //! "<KEY=VALUE>..." instead.
        template<auto... Annotations>
        static consteval ValueSpec Spec(std::meta::info member);
    };




    //! @brief Check if T is a @ref positional-vs-named "positional argument".
    template<class T, auto ...Annotations>
    concept IsPositionalConcept = std::ranges::contains(
        TypeParser<T, details_::internalArg>::template ExpectedFlags<Annotations...>(), std::string_view{});

    //! @brief Check if T is a @ref positional-vs-named "named only argument".
    template<class T, auto ...Annotations>
    concept IsNamedOnlyConcept = !IsPositionalConcept<T, Annotations...>;

    //! @copydoc IsPositionalConcept
    template<class T>
    consteval bool IsPositional(const std::span<std::meta::info const> annots) {
        std::vector args{ ^^T };
        args.append_range(annots);
        return std::meta::extract<bool>(std::meta::substitute(^^IsPositionalConcept, args));
    }
    //! @copydoc IsNamedOnlyConcept
    template<class T>
    consteval bool IsNamedOnly(const std::span<std::meta::info const> annots) {
        return !IsPositional<T>(annots);
    }

}

#include <Ogma/TypeParser/Help.hpp>
#include <Ogma/TypeParser/_base.tpp>
