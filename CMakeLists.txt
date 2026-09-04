cmake_minimum_required(VERSION 3.29.1)
project(
        MyProjectName
        VERSION 0.0.0
        DESCRIPTION "My Project Description"
        LANGUAGES CXX
)
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)

#region Settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_DEBUG_POSTFIX "_d")

set(PROJECT_VERSION_SUFFIX "")
set(PROJECT_VERSION_FULL "${PROJECT_VERSION}${PROJECT_VERSION_SUFFIX}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
#endregion

#region options
option(${PROJECT_NAME_UPPER}_BUILD_TESTS      "Build ${PROJECT_NAME} tests"         ${PROJECT_IS_TOP_LEVEL})
option(${PROJECT_NAME_UPPER}_BUILD_EXAMPLES   "Build ${PROJECT_NAME} examples"      ${PROJECT_IS_TOP_LEVEL})
option(${PROJECT_NAME_UPPER}_INSTALL          "Generate the install target."        ${PROJECT_IS_TOP_LEVEL})
option(${PROJECT_NAME_UPPER}_BUILD_BENCHMARKS "Build ${PROJECT_NAME} benchmarks"    OFF)
option(${PROJECT_NAME_UPPER}_BUILD_DOCS       "Build Doxygen documentation"         OFF)
#endregion

#region top-level compiler options
add_library(${PROJECT_NAME}CompilerFlags INTERFACE)
if(PROJECT_IS_TOP_LEVEL)
    if(MSVC)
        target_compile_options(${PROJECT_NAME}CompilerFlags INTERFACE
                $<IF:$<CONFIG:Release>,/permissive- /O2 /GL /Oi /Ot /GT /Gw /Gy /Zc:inline /openmp:llvm,>
        )
        target_link_options(${PROJECT_NAME}CompilerFlags INTERFACE $<IF:$<CONFIG:Release>,/LTCG,>)
    endif()
endif()
#endregion



#region extern aux files

enable_testing()
include(cmake/GetCPM.cmake)
include(cmake/Macros.cmake)
register_lldb_script(${PROJECT_NAME})

#endregion


add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
        POSITION_INDEPENDENT_CODE            ON
)

#region ${PROJECT_NAME} Files
file(GLOB_RECURSE ${PROJECT_NAME_UPPER}_PUBLIC_HEADERS CONFIGURE_DEPENDS include/${PROJECT_NAME}/*.hpp include/${PROJECT_NAME}/*.tpp)

target_sources(${PROJECT_NAME} PRIVATE
        # src/${PROJECT_NAME}/... your source files
        PUBLIC
        FILE_SET headers
        TYPE HEADERS
        BASE_DIRS include
        FILES ${${PROJECT_NAME_UPPER}_PUBLIC_HEADERS}
)
#endregion

#region linking

# add dependencies like:
#CPMFindPackage(
#        NAME                    Hermes
#        VERSION                 0.5.5
#        GITHUB_REPOSITORY       NotRiemannCousin/Hermes
#        GIT_TAG                 v0.5.5
#        OPTIONS                 "HERMES_INSTALL ${${PROJECT_NAME_UPPER}_INSTALL}"
#        FIND_PACKAGE_ARGUMENTS  "CONFIG"
#)
#
#CPMFindPackage(
#        NAME                    ZLIB
#        VERSION                 1.3.2
#        GITHUB_REPOSITORY       madler/zlib
#        GIT_TAG                 v1.3.2
#        OPTIONS                 "ZLIB_BUILD_EXAMPLES OFF"
#                                "ZLIB_BUILD_TESTING OFF"
#                                "ZLIB_BUILD_SHARED OFF"
#                                "ZLIB_INSTALL ${${PROJECT_NAME_UPPER}_INSTALL}"
#)
# If it isn't in a namespace:
#ensure_alias_target(zlibstatic ZLIB::ZLIB)
#
# If it also has a tools/*Lldb.py
#register_lldb_script(Hermes)
#
#target_link_libraries(${PROJECT_NAME} PUBLIC  Hermes::Hermes)
#target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

if(TARGET ${PROJECT_NAME}CompilerFlags)
    target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${PROJECT_NAME}CompilerFlags>)
endif()
#endregion


#region subdirectories

add_subdirectory_if(${PROJECT_NAME_UPPER}_BUILD_TESTS      tests)
add_subdirectory_if(${PROJECT_NAME_UPPER}_BUILD_EXAMPLES   examples)
add_subdirectory_if(${PROJECT_NAME_UPPER}_BUILD_BENCHMARKS benchmarks)
add_subdirectory_if(${PROJECT_NAME_UPPER}_BUILD_DOCS       docs)

#endregion


#region install
if(${PROJECT_NAME_UPPER}_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(${PROJECT_NAME_UPPER}_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}@${PROJECT_VERSION_FULL}")

    install(TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}Targets
            FILE_SET headers
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(EXPORT ${PROJECT_NAME}Targets
            FILE ${PROJECT_NAME}Targets.cmake
            NAMESPACE ${PROJECT_NAME}::
            DESTINATION ${${PROJECT_NAME_UPPER}_INSTALL_CMAKEDIR}
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            INSTALL_DESTINATION ${${PROJECT_NAME_UPPER}_INSTALL_CMAKEDIR}
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            DESTINATION ${${PROJECT_NAME_UPPER}_INSTALL_CMAKEDIR}
    )
endif()
#endregion


generate_lldbinit()