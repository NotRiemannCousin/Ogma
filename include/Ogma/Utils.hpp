#pragma once

//! @file Ogma/Utils.hpp
//! @brief Common types and reflection helpers used by Ogma.

#include <expected>
#include <ranges>
#include <format>
#include <meta>
#include <span>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Ogma {

    using ArgList = std::vector<std::string_view>;
    using ArgSpan = std::span<const std::string_view>;

    template<class T>
    using Result = std::expected<std::pair<T, ArgSpan>, std::string>;

    template<auto Info>
    using TypeOf = typename [: std::meta::type_of(Info) :];

    //! @brief A string with compile time static storage.
    //!
    //! ConstString promotes its value explicitly and remains implicitly convertible to @c std::string_view.
    struct ConstString {
        const char* ptr{};

        constexpr ConstString() = default;
        template<std::ranges::range R>
        consteval ConstString(R&& value)
            : ptr{ std::define_static_string(value) } {}
        consteval ConstString(const char* value)
            : ConstString(std::string_view{ value }) {}


        constexpr operator std::string_view() const {
            return ptr ? std::string_view{ ptr } : std::string_view{};
        }

        [[nodiscard]] constexpr auto begin() const noexcept { return std::string_view{ ptr }.begin(); }
        [[nodiscard]] constexpr auto end()   const noexcept { return std::string_view{ ptr }.end()  ; }

        [[nodiscard]] constexpr auto size()  const noexcept { return std::string_view{ ptr }.size() ; }
        [[nodiscard]] constexpr bool empty() const noexcept { return std::string_view{ ptr }.empty(); }

    };

    //! @brief The double-hyphen token from POSIX, repurposed as a general control token.
    //!
    //! In POSIX it terminates option parsing; here it "opts out" of the next token being read as a flag instead — a
    //! value that merely looks like one. In containers it opens a new element of named-only values; in positional
    //! strings it lets a value carry flag-shaped content, as in @c "-- --base64=idk" . It is not an end-of-options
    //! delimiter: parsing of the surrounding scope continues normally after it.
    constexpr std::string_view controlToken{ "--" };

    //! @brief The single-hyphen token that closes the current scope.
    //!
    //! The scope-based counterpart of the POSIX end-of-options delimiter: instead of ending option parsing globally,
    //! it exits exactly one level of nesting — a container, a nested structural member or an opened subcommand — and
    //! hands the remaining arguments back to the parent scope. It is optional whenever the argument list ends there
    //! anyway, and a trailing one at the root is harmless.
    constexpr std::string_view closingToken{ "-" };




    namespace Utils {

        constexpr bool IsLower(char c);
        constexpr bool IsUpper(char c);
        constexpr char ToLower(char c);
        constexpr char ToUpper(char c);

        constexpr auto isEmpty{ []<std::ranges::range R>(R&& str) { return std::ranges::empty(str); } };
        constexpr auto notEmpty{ std::not_fn(isEmpty) };


        constexpr auto joinBySpace{ std::views::join_with(std::string_view{ " "   }) };
        constexpr auto joinByComma{ std::views::join_with(std::string_view{ ", "  }) };
        constexpr auto joinByBar  { std::views::join_with(std::string_view{ " | " }) };

        constexpr auto concatBySpace{ std::views::filter(notEmpty) | joinBySpace | std::ranges::to<std::string>() };
        constexpr auto concatByComma{ std::views::filter(notEmpty) | joinByComma | std::ranges::to<std::string>() };
        constexpr auto concatByBar  { std::views::filter(notEmpty) | joinByBar   | std::ranges::to<std::string>() };



        constexpr auto asFlag{ [](const std::string_view str) { return std::string{ controlToken } + str; } };

        //! @brief Checks whether a member has exactly one requested annotation.
        template<std::meta::info ...Ts>
        consteval bool IsOneOf(std::meta::info ref);

        //! @brief Returns the only annotation of one of the requested types.
        template<std::meta::info ...Ts>
        consteval std::meta::info GetOf(std::meta::info ref);

        //! @brief Collects members together with their matching annotation.
        template<std::meta::info ...Ts>
        consteval std::vector<std::pair<std::meta::info, std::meta::info>>
        MatchedMembers(std::meta::info type);

        //! @brief Get the annotations of a member and convert it with `std::meta::constant_of` (from reflection of
        //! annotation to reflection of a value).
        consteval std::vector<std::meta::info> AnnotationValues(std::meta::info member);



        //! @brief Check if the token denotes a flag entry (a string starting with "--" with size greater than 2).
        //! @note This is different from @ref IsValidFlagName, this function only check "is the user trying to enter a
        //! flag?" when @ref IsValidFlagName checks "does the flag name (the string without the "--" part) specified
        //! has a valid name?".
        constexpr bool IsFlag(std::string_view token);

        //! @brief Checks if @p name is a valid flag name.
        //!
        //! A valid flag name is a non-empty ASCII string composed of lowercase letters, digits and hyphens, with no
        //! leading or trailing hyphen. This only validates the name spelling.
        //! @note This is different from @ref IsValidFlagName, this function only check "is the user trying to enter a
        //! flag?" when @ref IsValidFlagName checks "does the flag name (the string without the "--" part) specified
        //! has a valid name?".
        constexpr bool IsValidFlagName(std::string_view name);


        //! @brief Checks if @p templ is a specialisation of @p base.
        template<class T>
        consteval bool IsSpecializationOf(std::meta::info templ);

        //! @brief Returns the first annotation of type @p Tag.
        template<class Tag, auto... Annotations>
        consteval std::optional<Tag> GetAnnotation();


        template<auto Value>
        consteval std::meta::info ReflectValue();

        //! @brief Returns the annotations of @p Value that are @ref Fwd.
        template<auto... Annotations>
        consteval std::vector<std::meta::info> GetForwardedAnnots();


        //! @brief Convert a string to a number of type T using @c std::from_chars.
        //!
        //! @param args describes the arguments of @c std::from_chars.
        template<class T>
        constexpr std::expected<T, std::string> GetNumber(std::string_view value, auto... args);


        template<auto F>
        constexpr auto ConsumeOneAndForward(ArgSpan args) -> Result<std::remove_cvref_t<decltype(*std::invoke(F, args[0]))>>;
        template<class T>
        constexpr auto ConsumeOneAndForward(ArgSpan args);
    }

}

template<>
inline constexpr bool std::ranges::enable_borrowed_range<Ogma::ConstString> = true;

template<>
struct std::formatter<Ogma::ConstString> : std::formatter<std::string_view> {
    auto format(Ogma::ConstString value, std::format_context& context) const {
        return std::formatter<std::string_view>::format(value, context);
    }
};

#include <Ogma/Utils.tpp>
