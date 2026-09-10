#pragma once
#include <Ogma/TypeParser/_base.hpp>

namespace Ogma {

    //! @brief TypeParser specialization for @c bool.
    //!
    //! Parsing modes:
    //! @par Raw
    //! Activated with the `"--" + fieldName` or `"--no-" + fieldName` flag. The flag itself determines the value and
    //! this mode consumes no arguments. Like every option that consumes no arguments, it cannot be used as a positional
    //! option.
    //!
    //! @par Boolean
    //! Refers to @ref Ogma::BoolArg. Activated with @c `"--" + fieldName` and consumes either @c "true" or @c "false".
    //! Since it consumes a value, this mode can also be used as a positional argument.
    template<Arg ArgDefinition>
    struct TypeParser<bool, ArgDefinition> {
        //! @brief Parses the current boolean value according to the selected annotation mode.
        //!
        //! | Mode | Flag                                        | Consumes                |
        //! |------|---------------------------------------------|-------------------------|
        //! | Raw  | `"--" + fieldName` or `"--no-" + fieldName` | Nothing                 |
        //! | Bool | `"--" + fieldName`                          | @c "true" or @c "false" |
        template<auto... Annotations>
        static Result<bool> Parse(bool curr, std::string_view flag, ArgSpan args, std::string_view scopePath = {});

        //! @brief Reports the flag spellings and positional support of the selected bool mode.
        //!
        //! | Mode | ExpectedFlags                      |
        //! |------|------------------------------------|
        //! | Raw  | `{ fieldName, "no-" + fieldName }` |
        //! | Bool | `{ "", fieldName }`                |
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

#include <Ogma/TypeParser/_base/Bool.tpp>