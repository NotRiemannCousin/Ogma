#pragma once

//! @file Ogma/Annotations/Filesystem.hpp
//! @brief Annotations for filesystem path validation.

namespace Ogma {

    //! @brief Requires a path to refer to an existing regular file.
    //!
    //! @par Help
    //! Appended as "[file]".
    struct FileOnly {};

    //! @brief The annotation value used to enable regular file validation.
    inline constexpr FileOnly fileOnly{};

    //! @brief Requires a path to refer to an existing directory.
    //!
    //! @par Help
    //! Appended as "[dir]".
    struct DirOnly {};

    //! @brief The annotation value used to enable directory validation.
    inline constexpr DirOnly dirOnly{};

    //! @brief Requires a path to exist.
    //!
    //! @par Help
    //! Appended as "[must exist]".
    struct MustExist {};

    //! @brief The annotation value used to enable existence validation.
    inline constexpr MustExist mustExist{};

}
