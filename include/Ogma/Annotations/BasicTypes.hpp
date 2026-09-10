#pragma once

//! @file Ogma/Annotations/BasicTypes.hpp
//! @brief Core annotations for arguments, aliases, subcommands, and propagation.

#include <cstddef>
#include <Ogma/Utils.hpp>

namespace Ogma {

    //! @brief Gives a declaration an additional name.
    //!
    //! @par Help
    //! For enums, aliases are listed in the member's metavars alongside the enumerator identifiers. Every name the
    //! parser accepts is on the page, bar-separated, aliases after their enumerator.
    struct Alias {
        ConstString name{};
    };

    //! @brief Makes a boolean member consume an explicit "true" or "false" value.
    //! Without this annotation, a named bool is an on/off switch.
    //!
    //! @par Help
    //! With this annotation the metavariables render as "<TRUE|FALSE>". Without it, the raw bool shows no metavariables
    //! and its row documents both spellings compactly as "--[no-]name".
    struct BoolArg {};

    //! @brief The annotation value used to mark a boolean argument.
    inline constexpr BoolArg boolArg{};


    //! @brief Makes a repeated container option replace its current contents.
    //! Without this annotation, repeated containers accumulate values.
    struct Overwrite {};
    //! @brief The annotation value used to enable overwrite semantics.
    inline constexpr Overwrite overwrite{};

    //! @brief Makes a repeated container option append its current contents.
    //! This value is specified by consistency, but given that is the default in this library every class ignores it.
    struct Append {};
    //! @brief The annotation value used to enable append semantics.
    inline constexpr Append append{};

    //! @brief Makes enum parsing case insensitive.
    //!
    //! @par Help
    //! Set "[case insensitive]" for enums.
    struct CaseInsensitive {};

    //! @brief The annotation value used to enable case insensitive enum parsing.
    inline constexpr CaseInsensitive caseInsensitive{};

    //! @brief Forwards annotation values to the next nested parser level.
    template<auto...>
    struct Fwd {};
}
