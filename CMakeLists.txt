cmake_minimum_required(VERSION 3.20)

# ============================================================================
# PROJECT SETUP
# ============================================================================
project(art2img 
    VERSION 1.1.0
    DESCRIPTION "ART file to image converter library"
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/raulcorreia7/art2img"
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

# ============================================================================
# COMPILER CONFIGURATION
# ============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Enable parallel testing by default for local development
# CI/CD pipelines will override this with --parallel 1
set(CMAKE_CTEST_ARGUMENTS --parallel)

# Find Threads package - use native threads (Win32 on Windows, pthreads elsewhere)
find_package(Threads REQUIRED)

# ============================================================================
# DEPENDENCIES
# ============================================================================
set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/.cache")
include(cmake/get_cpm.cmake)

# STB image library - simple include directory approach
CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG fede005abaf93d9d7f3a679d1999b2db341b360f
)

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    CPMAddPackage(
        NAME doctest
        VERSION 2.4.12
        GITHUB_REPOSITORY "doctest/doctest"
        GIT_TAG "v2.4.12"
    )
endif()

option(BUILD_BENCHMARKS "Build benchmarks" OFF)
if(BUILD_BENCHMARKS)
    CPMAddPackage(
        NAME benchmark
        VERSION 1.8.3
        GITHUB_REPOSITORY "google/benchmark"
        GIT_TAG "v1.8.3"
    )
endif()

# ============================================================================
# TEST CONFIGURATION
# ============================================================================
# Platform-specific executable naming
if(WIN32)
    set(CLI_EXECUTABLE_NAME "art2img.exe")
else()
    set(CLI_EXECUTABLE_NAME "art2img")
endif()

# Test directory configuration
set(TEST_CLI_BINARY_DIR "${CMAKE_BINARY_DIR}/cli" CACHE PATH "CLI binary directory")
set(TEST_CLI_EXECUTABLE "${TEST_CLI_BINARY_DIR}/${CLI_EXECUTABLE_NAME}" CACHE PATH "CLI executable path")
set(TEST_OUTPUT_BASE "${CMAKE_BINARY_DIR}/test_output" CACHE PATH "Base test output directory")
set(TEST_ASSETS_DIR "${CMAKE_SOURCE_DIR}/tests/assets" CACHE PATH "Test assets directory")

# ============================================================================
# LIBRARY CREATION
# ============================================================================
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS "include/art2img/*.hpp")

add_library(libart2img ${LIB_SOURCES} ${LIB_HEADERS})
set_target_properties(libart2img PROPERTIES OUTPUT_NAME art2img)

# Compiler options - target-specific with generator expressions
target_compile_options(libart2img PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

# Link Threads library (handles pthread, win32 threads, etc.)
target_link_libraries(libart2img PRIVATE Threads::Threads)

# Optimization flags for Release build
target_compile_options(libart2img PRIVATE
    $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:MSVC>:/O2>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3 -DNDEBUG>
    >
)

target_include_directories(libart2img
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# STB integration - header-only library
if(stb_SOURCE_DIR)
    target_include_directories(libart2img SYSTEM PUBLIC ${stb_SOURCE_DIR})
    target_compile_definitions(libart2img PUBLIC STB_IMAGE_WRITE_IMPLEMENTATION)
endif()

# ============================================================================
# INSTALLATION
# ============================================================================
install(TARGETS libart2img
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/art2img
    DESTINATION include
)



# ============================================================================
# SUBDIRECTORIES
# ============================================================================
option(BUILD_CLI "Build CLI executable" ON)
if(BUILD_CLI)
    add_subdirectory(cli)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()