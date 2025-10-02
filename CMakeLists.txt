cmake_minimum_required(VERSION 3.14)
project(art2img VERSION 1.0.0 LANGUAGES CXX)

# CMake options
option(BUILD_CLI "Build the command line interface tool" ON)
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" ON)

# Enable testing at the top level
enable_testing()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set default install directories
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
endif()
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
    set(CMAKE_INSTALL_BINDIR bin)
endif()
if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR include)
endif()

# Set up build type specific flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific flags
if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /W4")
else()
    string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -pedantic -pthread")
endif()

# Optimization flags for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        string(APPEND CMAKE_CXX_FLAGS_RELEASE " /O2")
    else()
        string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O2")
    endif()
endif()

# Generate version header
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/version.hpp.in
    ${CMAKE_BINARY_DIR}/include/version.hpp
)

# Ensure include directories exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)

# Add vendor directory
include_directories(vendor)

# Find or download fmtlib
find_package(fmt QUIET)
if(NOT fmt_FOUND)
    # Try to find system fmt library
    find_library(FMT_LIB fmt)
    if(FMT_LIB)
        message(STATUS "Found system fmt library: ${FMT_LIB}")
        set(fmt_LIBRARIES ${FMT_LIB})
        set(fmt_FOUND TRUE)
    else()
        # Use the bundled fmt.hpp header
        message(STATUS "Using bundled fmt.hpp header")
        set(fmt_FOUND TRUE)
    endif()
endif()

# Include the source and generated header directories
include_directories(include)
include_directories(${CMAKE_BINARY_DIR}/include)

# Create the extractor library
add_library(art2img_extractor
    src/extractor_api.cpp
    src/art_file.cpp
    src/palette.cpp
    src/png_writer.cpp
    src/tga_writer.cpp
    src/exceptions.cpp
)

# Set up library properties and include directories
set_target_properties(art2img_extractor
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Define public interface - only expose API-relevant headers
target_include_directories(art2img_extractor
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

# Add header files as dependencies
target_sources(art2img_extractor
    PRIVATE
    include/extractor_api.hpp
    include/art_file.hpp
    include/palette.hpp
    include/png_writer.hpp
    include/tga_writer.hpp
    include/exceptions.hpp
    vendor/stb/stb_image_write.h
    ${CMAKE_BINARY_DIR}/include/version.hpp
)

# Set public headers for installation
set_target_properties(art2img_extractor
    PROPERTIES
    PUBLIC_HEADER "include/extractor_api.hpp;include/art_file.hpp;include/palette.hpp;include/exceptions.hpp"
)

# Include the cli tool
if (BUILD_CLI)
    add_subdirectory(cli)
endif()

# Build diagnostic tool if requested
if (BUILD_DIAGNOSTIC)
    add_executable(art2img_diagnostic
        src/diagnostic.cpp
    )

    target_include_directories(art2img_diagnostic
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor
        ${CMAKE_BINARY_DIR}/include
    )

    set_target_properties(art2img_diagnostic
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endif()

# Add doctest testing framework
if(BUILD_TESTS)
    find_package(doctest QUIET)

    if(TARGET doctest::doctest_with_main)
        message(STATUS "Found doctest: ${doctest_VERSION}")
    else()
        add_library(doctest INTERFACE)
        target_include_directories(doctest INTERFACE ${CMAKE_SOURCE_DIR}/third_party)

        add_library(doctest::doctest_with_main INTERFACE IMPORTED)
        target_link_libraries(doctest::doctest_with_main INTERFACE doctest)

        if(NOT TARGET doctest::doctest)
            add_library(doctest::doctest INTERFACE IMPORTED)
            target_link_libraries(doctest::doctest INTERFACE doctest)
        endif()

        message(STATUS "Using vendored doctest header")
    endif()

    add_subdirectory(tests)
endif()

# Include tests from subdirectory in the main test suite
if (BUILD_TESTS)
    # Set test working directory so tests can find assets
    set(TEST_WORKING_DIR ${CMAKE_BINARY_DIR})
endif()

# Installation targets
install(TARGETS art2img_extractor
    EXPORT art2img-extractor-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install the primary API header separately for easy inclusion
install(FILES include/extractor_api.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Generate and install pkg-config file if available
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/art2img-extractor.pc.in
        ${CMAKE_BINARY_DIR}/art2img-extractor.pc
        @ONLY
    )

    install(FILES ${CMAKE_BINARY_DIR}/art2img-extractor.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()
