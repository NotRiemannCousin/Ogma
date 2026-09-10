cmake_minimum_required(VERSION 3.29.1)
project(
        Ogma
        VERSION 0.0.0
        DESCRIPTION "A C++26 Reflection-based Argument Parser"
        LANGUAGES CXX
)

#region Settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_DEBUG_POSTFIX "_d")

set(PROJECT_VERSION_SUFFIX "")
set(PROJECT_VERSION_FULL "${PROJECT_VERSION}${PROJECT_VERSION_SUFFIX}")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
#endregion

#region options
option(OGMA_BUILD_TESTS      "Build Ogma tests"                ${PROJECT_IS_TOP_LEVEL})
option(OGMA_BUILD_EXAMPLES   "Build Ogma examples"             ${PROJECT_IS_TOP_LEVEL})
option(OGMA_INSTALL          "Generate the install target."    ${PROJECT_IS_TOP_LEVEL})
option(OGMA_BUILD_BENCHMARKS "Build Ogma benchmarks"           OFF)
option(OGMA_BUILD_DOCS       "Build Doxygen documentation"     OFF)
#endregion

#region top-level compiler options
add_library(OgmaCompilerFlags INTERFACE)
if(PROJECT_IS_TOP_LEVEL)
    if(MSVC)
        target_compile_options(OgmaCompilerFlags INTERFACE
                $<IF:$<CONFIG:Release>,/permissive- /O2 /GL /Oi /Ot /GT /Gw /Gy /Zc:inline /openmp:llvm,>
        )
        target_link_options(OgmaCompilerFlags INTERFACE $<IF:$<CONFIG:Release>,/LTCG,>)
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(OgmaCompilerFlags INTERFACE -freflection)
endif()
#endregion



#region extern aux files

enable_testing()
include(cmake/GetCPM.cmake)
include(cmake/Macros.cmake)
register_lldb_script(Ogma)

#endregion

add_library(Ogma INTERFACE)
add_library(Ogma::Ogma ALIAS Ogma)

target_include_directories(Ogma INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

#region Ogma Files
file(GLOB_RECURSE OGMA_PUBLIC_HEADERS CONFIGURE_DEPENDS include/Ogma/*.hpp include/Ogma/*.tpp)

target_sources(Ogma INTERFACE
        # src/Ogma/... your source files
        PUBLIC
        FILE_SET headers
        TYPE HEADERS
        BASE_DIRS include
        FILES ${OGMA_PUBLIC_HEADERS}
)
#endregion

#region linking
if(TARGET OgmaCompilerFlags)
    target_link_libraries(Ogma INTERFACE $<BUILD_INTERFACE:OgmaCompilerFlags>)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_link_libraries(Ogma INTERFACE stdc++exp)
endif()
#endregion


#region subdirectories

add_subdirectory_if(OGMA_BUILD_TESTS      tests)
add_subdirectory_if(OGMA_BUILD_EXAMPLES   examples)
add_subdirectory_if(OGMA_BUILD_BENCHMARKS benchmarks)
add_subdirectory_if(OGMA_BUILD_DOCS       docs)

#endregion


#region install
if(OGMA_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(OGMA_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Ogma@${PROJECT_VERSION_FULL}")

    install(TARGETS Ogma
            EXPORT OgmaTargets
            FILE_SET headers
    )

    install(EXPORT OgmaTargets
            FILE OgmaTargets.cmake
            NAMESPACE Ogma::
            DESTINATION ${OGMA_INSTALL_CMAKEDIR}
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/OgmaConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/OgmaConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/OgmaConfig.cmake"
            INSTALL_DESTINATION ${OGMA_INSTALL_CMAKEDIR}
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/OgmaConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/OgmaConfigVersion.cmake"
            DESTINATION ${OGMA_INSTALL_CMAKEDIR}
    )
endif()
#endregion


generate_lldbinit()