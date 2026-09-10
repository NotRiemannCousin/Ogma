#pragma once
#include <Ogma/TypeParser/_base.hpp>
#include <type_traits>

namespace Ogma {

    //! @brief TypeParser specialization for scoped and unscoped enum types.
    //!
    //! Matches an enumerator identifier or an @ref Alias annotation. You can control the ase sensitivity by specifying
    //! @ref CaseInsensitive on the enum type or in the `T` member itself.
    //! This type is @ref positional-vs-named "positional-capable".
    template<class T, Arg ArgDefinition>
        requires std::is_enum_v<T>
    struct TypeParser<T, ArgDefinition> {
        //! @brief Parse an enum value from @p args.
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

#include <Ogma/TypeParser/_base/Enum.tpp>
