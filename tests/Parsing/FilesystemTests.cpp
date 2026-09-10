// ReSharper disable CppDeclaratorNeverUsed
#include <Ogma/Parser.hpp>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#pragma region Basic constraints

// ===================================================================================================================
// Just like std::filesystem::path, it will not check if is a valid path.
// ===================================================================================================================

namespace fs = std::filesystem;

namespace {
    struct PathOptions {
        [[ =Ogma::Arg{ "existing"  }, =Ogma::Fwd<Ogma::mustExist>() ]] std::optional<fs::path> existing{};
        [[ =Ogma::Arg{ "file"      }, =Ogma::Fwd<Ogma::fileOnly>()  ]] std::optional<fs::path> file{};
        [[ =Ogma::Arg{ "directory" }, =Ogma::Fwd<Ogma::dirOnly>()   ]] std::optional<fs::path> directory{};
    };
}


TEST(Filesystem, AcceptsMatchingPathKinds) {
    // Verifies all fields.
    const auto dir{ fs::temp_directory_path() };
    const auto file{ dir / "ogma_filesystem_test_file.tmp" };

    { std::ofstream out{ file }; out << "ogma"; }

    const auto parsed{ Ogma::Parse<PathOptions>({{
        "--existing" , dir.string(),
        "--file"     , file.string(),
        "--directory", dir.string()
    }}) };

    ASSERT_TRUE(parsed.has_value()) << parsed.error();
    EXPECT_EQ(parsed->first.existing , dir);
    EXPECT_EQ(parsed->first.file     , file);
    EXPECT_EQ(parsed->first.directory, dir);

    fs::remove(file);
}


TEST(Filesystem, FailsWhenPathArgumentIsMissing) {
    // Path can't be empty.
    const auto parsed{ Ogma::Parse<PathOptions>({{ "--existing" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("expected a path"), std::string::npos);
}

#pragma endregion



#pragma region Existence Constraints

// ===================================================================================================================
// The file must at least exist.
// ===================================================================================================================

TEST(Filesystem, FailsWhenMustExistDoesNotExist) {
    // Ensuring Ogma::mustExist existence.
    const auto parsed{ Ogma::Parse<PathOptions>({{ "--existing", "/path/that/definitely/does/not/exist/9999" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("path does not exist"), std::string::npos);
}
TEST(Filesystem, FailsWhenFileOnlyDoesNotExist) {
    // Ensuring Ogma::fileOnly existence.
    const auto parsed{ Ogma::Parse<PathOptions>({{ "--file", "/path/that/definitely/does/not/exist/9999.kath" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("path is not a regular file"), std::string::npos);
}
TEST(Filesystem, FailsWhenDirOnlyDoesNotExist) {
    // Ensuring Ogma::dirOnly existence.
    const auto parsed{ Ogma::Parse<PathOptions>({{ "--directory", "/path/that/definitely/does/not/exist/9999/" }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("path is not a directory"), std::string::npos);
}

#pragma endregion



#pragma region FileType Constraints

// ===================================================================================================================
// Check if is a file/folder.
// ===================================================================================================================

TEST(Filesystem, FailsWhenFileOnlyReceivesDirectory) {
    // Ensuring Ogma::fileOnly.
    const auto dir{ fs::temp_directory_path() };
    const auto parsed{ Ogma::Parse<PathOptions>({{ "--file", dir.string() }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("path is not a regular file"), std::string::npos);
}

TEST(Filesystem, FailsWhenDirOnlyReceivesFile) {
    // Ensuring Ogma::dirOnly.
    const auto dir{ fs::temp_directory_path() };
    const auto file{ dir / "ogma_filesystem_not_a_dir.tmp" };
    { std::ofstream out{ file }; out << "ogma"; }

    const auto parsed{ Ogma::Parse<PathOptions>({{ "--directory", file.string() }}) };

    ASSERT_FALSE(parsed.has_value());
    EXPECT_NE(parsed.error().find("path is not a directory"), std::string::npos);

    fs::remove(file);
}

#pragma endregion