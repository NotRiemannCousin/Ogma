#pragma once
#include <Ogma/TypeParser/_base.hpp>
#include <optional>

namespace Ogma {

    //! @brief TypeParser specialization for @c std::optional<T>.
    //!
    //! Delegates value parsing to the TypeParser of the underlying @c T and wraps the result in a @c std::optional<T>.
    //! Is a @ref positional-vs-named "named-only value".
    //!
    //! @attention This is the only really "optional" type in the recursive @c TypeParser engine.
    //!
    //! It's the only type that can be omitted from the parsing process from the default recursive @c TypeParser (for
    //! classes). Other named-only values are required just like positional values. If you need an value omitted, you
    //! can wrap it in a @c std::optional.
    //!
    //! This parser don't write @c std::nullopt, so if the default value is @c std::nullopt (like in
    //! `std::optional<std::FormatEnum> format{};`) and the field end's up being @c std::nullopt then it's because the
    //! flag just wasn't specified. You can use this to know if the flag was passed and make guards like
    //! `"No default format selected. Continue?"` or using `.value_or(FormaEnum::Mp4)` to get a @c FormaEnum with a
    //! default directly. You can always use `std::optional<std::FormatEnum> format{ FormaEnum::Mp4 };` too.
    template<class T, Arg ArgDefinition>
    struct TypeParser<std::optional<T>, ArgDefinition> {
        //! @brief Parses an @c T value and replaces the current optional contents.
        template<auto... Annotations>
        static Result<std::optional<T>> Parse(std::optional<T> curr, std::string_view flag, ArgSpan args, std::string_view scopePath = {});

        //! @returns `{ fieldName }`.
        template<auto... Annotations>
        static constexpr ArgSpan ExpectedFlags();

        //! @brief Reports how the value after the flag is consumed.
        //!
        //! Delegates to the inner type's ValueSpec through the same forwarding rules as Parse, so the usage fragment
        //! stays in sync with what the parser accepts (an optional integer shows INT, etc.). The omission brackets are
        //! the member-walker's job, not the type's.
        template<auto... Annotations>
        static consteval ValueSpec Spec(std::meta::info member);
    };

}

#include <Ogma/TypeParser/_base/Optional.tpp>
