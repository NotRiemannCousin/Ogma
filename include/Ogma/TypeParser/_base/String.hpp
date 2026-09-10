#pragma once
#include <Ogma/TypeParser/_base.hpp>

namespace Ogma {

    //! @brief TypeParser specialization for @c std::string.
    //!
    //! Consumes the next token verbatim as a UTF-8 string. When set as a positional arg (empty @c flag in the @c Parse
    //! function), you can pass a "--" string that will be ignored in order to pass strings that starts with "--" and
    //! could conflict with the flags (just like "--" works in POSIX).
    //!
    //! This type is @ref positional-vs-named "positional-capable".
    template<Arg ArgDefinition>
    struct TypeParser<std::string, ArgDefinition> {
        //! @brief Parse a string from @p args. If surrounded by `"--"` the inner string is selected.
        template<auto... Annotations>
        static Result<std::string> Parse(std::string curr, std::string_view flag, ArgSpan args, std::string_view scopePath = {});

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

#include <Ogma/TypeParser/_base/String.tpp>
