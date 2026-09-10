#pragma once
#include <string>
#include <Ogma/Utils.hpp>

namespace Ogma {

#pragma region Annotations

#pragma region Arg

    //! @brief The pivot annotation from this library. Describes a named argument.
    //!
    //! @par Help
    //! @c description is the member's prose in the help page. The parser's own @c ValueSpec modifiers ("[format: hex]",
    //! "[range: 0..100]") are appended after it automatically.
    //!
    //! @c hidden removes the member from the page entirely.
    struct Arg {
        ConstString longName{};
        ConstString description{};
        bool hidden{ false };
    };

    namespace details_ { constexpr auto internalArg{ Arg{ "_name_" } }; }

    //! @brief Extracts @p Mem's @ref Arg annotation value. Behaviour is undefined if @p Mem
    //! is not annotated with @ref Arg.
    template<auto Mem>
    consteval Arg GetArg();

#pragma endregion



#pragma region Subcommand

    //! @brief Describes the branches of a variant-backed subcommand.
    //!
    //! A bare token naming one of the @c cases opens the branch, which then owns every following token exactly like any
    //! other scope: the closing token ends it and hands the remainder back to the caller. The member must be the last
    //! annotated member of its struct — bare tokens fill positional members in declaration order, so anything declared
    //! after the subcommand would never be reachable by position. Members that must not compete for bare tokens go into
    //! std::optional (filled by name); positional members of a subcommand level belong inside the branch types
    //! themselves.
    //!
    //! Entering a branch opens the same kind of scope a nested struct does, see @ref scopes "Scopes".
    struct VariantArg {
        struct VariantBranch {
            ConstString longName{};
            ConstString description{};
        };

        const VariantBranch* cases{};
        std::size_t caseCount{};
    };

#pragma endregion



    //! @brief Metadata for the root application type.
    struct AppInfo {
        ConstString name{};
        ConstString version{};
        ConstString description{};
    };

    //! @brief True when @p T (the root config struct) carries an @ref AppInfo annotation.
    template<class T>
    consteval bool HasAppInfo();

#pragma endregion


#pragma region Help

    //! @brief Exception thrown by the struct-walking engine when a scope intercepts "--help" or "--version". Carries
    //! the page exactly as HelpText renders it.
    struct HelpRequested {
        std::string text{};
    };




    //! @brief Struct used to show the help information about a flag.
    struct ValueSpec {
        //! Flag shown in usage.
        ConstString usageFlag{};
        //! Used to show the flags that can be matched. Don't need to be a valid flag (like `"--[no-]flag-name"`).
        ConstString flagList{};
        //! Used to show the semantic names that the user should expect. The `"<NAME>"` or `"<COUNT>"` thing.
        ConstString metavars{};
        //! Used to show the info about the annotations to the user.
        ConstString modifiers{};
    };

    //! @brief Create a scope path from a Type. If T has @ref AppInfo, the scope is the app name, otherwise is the T
    //! identifier.
    template<class T> consteval std::string_view ScopePathOf();

    //! @brief Formats and returns the version.
    template<class T> std::string RenderVersion();

#pragma endregion

}

#include <Ogma/_base.tpp>