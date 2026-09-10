#pragma once
#include <Ogma/TypeParser/_base.hpp>

#include <map>
#include <string>

namespace Ogma {

    //! @brief TypeParser specialisation for @c "key=value" options.
    //!
    //! @note Only `std::map<std::string, std::string>` is supported by this specialisation.
    //!
    //! @par Key/value
    //! Each token is a self-contained @c "key=value" pair. A repeated key replaces its previous value.
    //! This type is @ref positional-vs-named "positional-capable".
    //!
    //! @par Accumulation x Overwrite
    //! Repeated occurrences accumulate. Add @ref Overwrite to clear the existing container first.
    template<class Compare, class Alloc, Arg ArgDefinition>
    struct TypeParser<std::map<std::string, std::string, Compare, Alloc>, ArgDefinition> {
        using Map    = std::map<std::string, std::string, Compare, Alloc>;
        using MapRes = Result<Map>;

        template<auto... Annotations>
        static auto Parse(Map curr, std::string_view, ArgSpan args, std::string_view scopePath = {}) -> MapRes;

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

#include <Ogma/TypeParser/_base/Map.tpp>
