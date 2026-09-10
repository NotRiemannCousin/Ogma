#pragma once

//! @file Ogma/TypeParser/Help.hpp
//! @brief Help rendering for parsed configuration types.

#include <Ogma/Parser.hpp>

#include <string>
#include <string_view>

namespace Ogma {
    //! @brief Formats and returns the help page.
    template<class T> std::string RenderHelpPage(std::string_view scopePath = {});

    //! @brief Renders the help page of @p T's scope.
    //!
    //! @param scopePath how the scope is addressed in the usage line. Empty means the root.
    //!
    //! Hidden members never appear. The usage line and options formatting are driven entirely by the @c ValueSpec
    //! returned by each member's @c TypeParser.
    //!
    //! The page anatomy and the usage grammar are documented in @ref help-page "Help", and the extension point
    //! that feeds it in @ref type-parser "TypeParser structure".
    template<class T>
    std::string HelpText(std::string_view scopePath = {});

}