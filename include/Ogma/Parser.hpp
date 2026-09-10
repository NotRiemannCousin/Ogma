#pragma once

//! @file Ogma/Parser.hpp
//! @brief Public declarations for the Ogma command line parser.
//!
//! Template definitions are provided by @c <Ogma/Parser.tpp>, which is included
//! by this header so users only need to include one file.

#include <Ogma/Annotations/_pch.hpp>
#include <Ogma/TypeParser/_base.hpp>
#include <Ogma/Utils.hpp>

#include <expected>
#include <string>

namespace Ogma {

    //! @brief Parses all @ref Arg and @ref VariantArg annotated members of @p T.
    //!
    //! Named options are matched by flag spelling. Every member's own @c TypeParser::ExpectedFlags (minus the
    //! positional @c "" entry) feeds a constexpr flag table, and a "--"-shaped token is looked up there. Members that
    //! accept a positional invocation are parsed in declaration order, a bare token fills the first still-unfilled
    //! positional entry, and a @ref VariantArg member matches the subcommand names declared in its @c cases. Both paths
    //! go through the same per-member setter, so repeated named occurrences keep accumulating (or overwriting) exactly
    //! as each member's own parser decides, and a member reached positionally cannot be claimed twice.
    //!
    //! A plain @c Arg annotated member whose own type is itself an unspecialized struct with Arg/VariantArg members
    //! recurses automatically. There is no separate dispatch for it, it simply reaches
    //! @c TypeParser<FieldT>::Parse like any other member, and that type's primary (unspecialized) fallback IS the same
    //! struct-walking engine used at every level, root or nested alike. Such a member is entered through one of its own
    //! ExpectedFlags spellings (or, when its type is positional-capable, directly by a bare token), after which its own
    //! flags and positional members are recognised in place. An explicit closing token (@ref Ogma::closingToken) ends
    //! that nested member's own token stream and hands the remainder back to its caller.
    //!
    //! Any input left over once @p T is fully parsed, including a stray, unconsumed closing token, is reported as an
    //! error.
    //!
    //! A "--help" token is intercepted by the scope that sees it (and "--version" too, when the root declares
    //! @ref AppInfo), which aborts parsing by throwing @ref HelpRequested carrying the rendered page. Catch it if you
    //! drive @c Parse directly, or use @ref ParseAndExecute which already handles it.
    //!
    //! The token-level rules (positional queue, named table, subcommand dispatch) are documented in
    //! @ref positional-vs-named "Positional vs Named Arguments", and the nesting rules in @ref scopes "Scopes".
    template<class T>
    Result<T> Parse(ArgSpan args);

    //! @brief Executes a parsed configuration object.
    //!
    //! A callable configuration is invoked directly. A variant configuration is visited recursively until a callable
    //! branch is reached. A plain struct without an operator() and without any variant member has nothing to run and
    //! returns 0.
    template<class T>
    std::expected<int, std::string> Execute(const T& config);

    //! @brief Parses @p args into @p T and immediately executes the result.
    //!
    //! Errors are printed to standard error and produce @c EXIT_FAILURE. A "--help" or "--version" request prints its
    //! page to standard output and counts as success, which is why the empty-error path returns EXIT_SUCCESS instead
    //! of failing.
    template<class T>
    int ParseAndExecute(ArgSpan args);

}

#include <Ogma/Parser.tpp>