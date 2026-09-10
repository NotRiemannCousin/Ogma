#pragma once
#include <Ogma/TypeParser/_base.hpp>

#include <set>
#include <unordered_set>
#include <vector>

namespace Ogma {

    namespace details_ {
        //! @par Raw
        //! Parsing continues until a flag (a string starting with "--*") or a closing token (the string "-") is found,
        //! or until the @ref ArgList is empty. If @c T is a @ref positional-vs-named "named-only" argument each element
        //! requires a @c "--" string to initiate a new element.
        //!
        //! @note The reason named-only arguments cannot be specified positionally is that they lack start delimiters,
        //! and the field flags serve to make this explicit; "--" performs that same role here.
        //!
        //! @par Accumulation x Overwrite
        //! Repeated occurrences accumulate. Add @ref Overwrite to clear the existing container first.
        //!
        struct CollectionParser {
            //! @brief Reads a sequential list of elements delimited by a flag (a string starting with "--*"), a closing
            //! token (the string "-") or by the @ref ArgList count (while not emtpy). If @c T is a
            //! @ref positional-vs-named "named-only" argument each element requires a @c "--" string to initiate a new
            //! element.
            static void Parse(){}

            //! @returns `{ "", fieldName }`.
            static void ExpectedFlags(){}

            //! @brief Reports how the member shows up in the help page.
            //!
            //! Fill this together with @c Parse and @c ExpectedFlags, a specialisation that changes what it accepts per
            //! annotation (like the bool modes selected by @ref Ogma::BoolArg) should change this per annotation too.
            static void Spec(){}
        };
    }




    //! @brief TypeParser specialisation for a std::vector.
    //!
    //! @copydetails Ogma::details_::CollectionParser
    template<class T, Arg ArgDefinition>
    struct TypeParser<std::vector<T>, ArgDefinition> {
        //! @cond
        using Vector    = std::vector<T>;
        using VectorRes = Result<Vector>;
        //! @endcond

        //! @copydoc Ogma::details_::CollectionParser::Parse()
        //! @static
        template<auto... Annotations>
        static auto Parse(Vector curr, std::string_view, ArgSpan args, std::string_view scopePath = {}) -> VectorRes;

        //! @copydoc Ogma::details_::CollectionParser::ExpectedFlags()
        //! @static
        template<auto... Annotations>
        static constexpr ArgSpan ExpectedFlags();

        //! @copydoc Ogma::details_::CollectionParser::Spec()
        template<auto... Annotations>
        static consteval ValueSpec Spec(std::meta::info member);
    };

    //! @brief TypeParser specialisation for @c std::set.
    //!
    //! @copydetails Ogma::details_::CollectionParser
    template<class T, class Compare, Arg ArgDefinition>
    struct TypeParser<std::set<T, Compare>, ArgDefinition> {
        //! @cond
        using Set    = std::set<T, Compare>;
        using SetRes = Result<Set>;
        //! @endcond

        //!  @copydoc Ogma::details_::CollectionParser::Parse()
        template<auto... Annotations>
        static auto Parse(Set curr, std::string_view, ArgSpan args, std::string_view scopePath = {}) -> SetRes;

        //! @copydoc Ogma::details_::CollectionParser::ExpectedFlags()
        template<auto... Annotations>
        static constexpr ArgSpan ExpectedFlags();

        //! @copydoc Ogma::details_::CollectionParser::Spec()
        template<auto... Annotations>
        static consteval ValueSpec Spec(std::meta::info member);
    };

    //! @brief TypeParser specialisation for @c std::unordered_set.
    //!
    //! @copydetails Ogma::details_::CollectionParser
    template<class T, class Hash, class Eq, Arg ArgDefinition>
    struct TypeParser<std::unordered_set<T, Hash, Eq>, ArgDefinition> {
        //! @cond
        using UnorderedSet    = std::unordered_set<T, Hash, Eq>;
        using UnorderedSetRes = Result<UnorderedSet>;
        //! @endcond

        //! @copydoc Ogma::details_::CollectionParser::Parse()
        template<auto... Annotations>
        static auto Parse(UnorderedSet curr, std::string_view, ArgSpan args, std::string_view scopePath = {}) -> UnorderedSetRes;

        //! @copydoc Ogma::details_::CollectionParser::ExpectedFlags()
        template<auto... Annotations>
        static constexpr ArgSpan ExpectedFlags();

        //! @copydoc Ogma::details_::CollectionParser::Spec()
        template<auto... Annotations>
        static consteval ValueSpec Spec(std::meta::info member);
    };

}

#include <Ogma/TypeParser/_base/Containers.tpp>
