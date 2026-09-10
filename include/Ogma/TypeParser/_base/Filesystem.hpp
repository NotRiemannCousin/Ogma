#pragma once
#include <Ogma/TypeParser/_base.hpp>
#include <filesystem>

namespace Ogma {

    //! @brief TypeParser specialization for @c std::filesystem::path.
    //!
    //! Constructs the path from the next token. You can specify some common checks using @ref MustExist, @ref FileOnly
    //! and @ref DirOnly.
    //! This type is @ref positional-vs-named "positional-capable".
    template<Arg ArgDefinition>
    struct TypeParser<std::filesystem::path, ArgDefinition> {
        //! @brief Parse a filesystem path from @p args.
        template<auto... Annotations>
        static Result<std::filesystem::path> Parse(std::filesystem::path curr, std::string_view, ArgSpan args, std::string_view scopePath = {});

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

#include <Ogma/TypeParser/_base/Filesystem.tpp>
