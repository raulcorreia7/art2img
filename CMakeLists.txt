cmake_minimum_required(VERSION 3.14)
project(art2img VERSION 0.1.0 LANGUAGES CXX)

# Always generate compile_commands.json to support tooling such as clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable compile_commands.json generation" FORCE)

# Set default parallel build jobs to use all available CPUs if not explicitly set
if(NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    include(ProcessorCount)
    ProcessorCount(PROCESSOR_COUNT)
    if(PROCESSOR_COUNT GREATER 1)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${PROCESSOR_COUNT})
    else()
        set(CMAKE_BUILD_PARALLEL_LEVEL 1)
    endif()
endif()

# CMake options
option(BUILD_CLI "Build the command line interface tool" ON)
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" ON)
option(BUILD_DIAGNOSTIC "Build diagnostic tool" OFF)

# Enable testing at the top level
enable_testing()

# CPM Package Manager
include(cmake/CPM.cmake)

# Dependencies via CPM
CPMFindPackage(
    NAME CLI11
    GITHUB_REPOSITORY CLIUtils/CLI11
    GIT_TAG v2.5.0
)

CPMFindPackage(
    NAME doctest
    GITHUB_REPOSITORY doctest/doctest
    GIT_TAG v2.4.12
)

CPMFindPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG fede005abaf93d9d7f3a679d1999b2db341b360f  # Latest commit on master branch
)

CPMFindPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 11.0.2
)

# Create interface library for stb since CPM doesn't create it automatically
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE ${stb_SOURCE_DIR})
# Suppress warnings from stb headers
set_target_properties(stb_headers PROPERTIES
    INTERFACE_COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:CXX>:-w>"
)
add_library(stb::headers ALIAS stb_headers)



# Create interface libraries for doctest if CPM doesn't create the expected targets
if(BUILD_TESTS)
    if(NOT TARGET doctest)
        add_library(doctest INTERFACE)
        target_include_directories(doctest INTERFACE ${doctest_SOURCE_DIR})
        # Suppress warnings from doctest
        set_target_properties(doctest PROPERTIES
            INTERFACE_COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:CXX>:-w>"
        )
    endif()

    if(NOT TARGET doctest::doctest_with_main)
        add_library(doctest::doctest_with_main INTERFACE IMPORTED)
        target_link_libraries(doctest::doctest_with_main INTERFACE doctest)
        # Suppress warnings from doctest
        set_target_properties(doctest::doctest_with_main PROPERTIES
            INTERFACE_COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:CXX>:-w>"
        )
    endif()

    if(NOT TARGET doctest::doctest)
        add_library(doctest::doctest INTERFACE IMPORTED)
        target_link_libraries(doctest::doctest INTERFACE doctest)
        # Suppress warnings from doctest
        set_target_properties(doctest::doctest PROPERTIES
            INTERFACE_COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:CXX>:-w>"
        )
    endif()
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -pedantic")
    # Add pthread for non-MSVC compilers (Linux, MinGW)
    if(NOT WIN32)
        string(APPEND CMAKE_CXX_FLAGS " -pthread")
    endif()
endif()

# Sanitizer options
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_LEAK_SANITIZER "Enable LeakSanitizer" OFF)

if(ENABLE_ASAN)
    if(NOT MSVC AND NOT WIN32)
        set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g")
        string(APPEND CMAKE_CXX_FLAGS " ${SANITIZER_FLAGS}")
        string(APPEND CMAKE_EXE_LINKER_FLAGS " ${SANITIZER_FLAGS}")
    endif()
endif()

if(ENABLE_LEAK_SANITIZER)
    if(NOT MSVC AND NOT WIN32)
        set(LEAK_SANITIZER_FLAGS "-fsanitize=leak -fno-omit-frame-pointer -g")
        string(APPEND CMAKE_CXX_FLAGS " ${LEAK_SANITIZER_FLAGS}")
        string(APPEND CMAKE_EXE_LINKER_FLAGS " ${LEAK_SANITIZER_FLAGS}")
    endif()
endif()

# Optimization flags for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        string(APPEND CMAKE_CXX_FLAGS_RELEASE " /O2")
    else()
        string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -DNDEBUG -flto=auto -ffast-math")
        string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto=auto")
        # Architecture-specific optimizations for x86_64
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
            string(APPEND CMAKE_CXX_FLAGS_RELEASE " -march=x86-64 -mtune=generic")
        endif()
    endif()
endif()

# Generate version header
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/version.hpp.in
    ${CMAKE_BINARY_DIR}/include/version.hpp
)

# Ensure include directories exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)

# Create the extractor library
add_library(art2img_extractor
    src/extractor_api.cpp
    src/art_file.cpp
    src/palette.cpp
    src/image_writer.cpp
    src/image_processor.cpp
    src/exceptions.cpp
    src/file_operations.cpp
    src/colors.cpp
)

# Set C++ standard via compile features
target_compile_features(art2img_extractor
    PUBLIC
        cxx_std_20
)

# Suppress warnings from external dependencies
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(art2img_extractor PRIVATE
        "-Wno-format-truncation"
    )
endif()

# Set up library properties
set_target_properties(art2img_extractor
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Define public interface - only expose API-relevant headers
target_include_directories(art2img_extractor
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add header files as dependencies
target_sources(art2img_extractor
    PRIVATE
    include/extractor_api.hpp
    include/art_file.hpp
    include/palette.hpp
    include/image_writer.hpp
    include/image_processor.hpp
    include/exceptions.hpp
    include/file_operations.hpp
    ${CMAKE_BINARY_DIR}/include/version.hpp
)

target_link_libraries(art2img_extractor
    PRIVATE
        stb::headers
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
        ${CMAKE_BINARY_DIR}/include
    )

    target_compile_features(art2img_diagnostic
        PRIVATE
            cxx_std_20
    )

    target_link_libraries(art2img_diagnostic
        PRIVATE
        art2img_extractor
    )

    set_target_properties(art2img_diagnostic
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

if(BUILD_TESTS)
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

# Collect project source files for tooling integrations
set(ART2IMG_TOOLING_SOURCE_GLOBS
    ${CMAKE_SOURCE_DIR}/include/*.hpp
    ${CMAKE_SOURCE_DIR}/include/*.h
    ${CMAKE_SOURCE_DIR}/include/*.cpp
    ${CMAKE_SOURCE_DIR}/include/*.c
    ${CMAKE_SOURCE_DIR}/src/*.hpp
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/cli/*.hpp
    ${CMAKE_SOURCE_DIR}/cli/*.h
    ${CMAKE_SOURCE_DIR}/cli/*.cpp
    ${CMAKE_SOURCE_DIR}/cli/*.cc
    ${CMAKE_SOURCE_DIR}/cli/*.cxx
    ${CMAKE_SOURCE_DIR}/tests/*.hpp
    ${CMAKE_SOURCE_DIR}/tests/*.h
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_SOURCE_DIR}/tests/*.c
)

file(GLOB_RECURSE ART2IMG_ALL_TOOLING_SOURCES
    CONFIGURE_DEPENDS
    ${ART2IMG_TOOLING_SOURCE_GLOBS}
)

file(GLOB_RECURSE ART2IMG_ALL_TRANSLATION_UNITS
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.cc
    ${CMAKE_SOURCE_DIR}/src/*.cxx
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/cli/*.c
    ${CMAKE_SOURCE_DIR}/cli/*.cc
    ${CMAKE_SOURCE_DIR}/cli/*.cxx
    ${CMAKE_SOURCE_DIR}/cli/*.cpp
    ${CMAKE_SOURCE_DIR}/tests/*.c
    ${CMAKE_SOURCE_DIR}/tests/*.cc
    ${CMAKE_SOURCE_DIR}/tests/*.cxx
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
)

# Install vendored dependencies that need to be available to consumers
install(FILES include/BS_thread_pool.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Add clang-format targets
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(clang-format
        COMMAND ${CLANG_FORMAT_EXE} -i ${ART2IMG_ALL_TOOLING_SOURCES}
        COMMENT "Formatting all C++ source files with clang-format"
        VERBATIM
    )

    add_custom_target(clang-format-dry-run
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${ART2IMG_ALL_TOOLING_SOURCES}
        COMMENT "Checking formatting of all C++ source files with clang-format"
        VERBATIM
    )
else()
    add_custom_target(clang-format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found; install clang-format to enable formatting support."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "clang-format not available"
    )

    add_custom_target(clang-format-dry-run
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found; install clang-format to enable formatting support."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "clang-format not available"
    )
endif()

# Add clang-tidy lint target
find_program(RUN_CLANG_TIDY_EXE NAMES run-clang-tidy run-clang-tidy.py)
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(RUN_CLANG_TIDY_EXE)
    add_custom_target(clang-tidy
        COMMAND ${RUN_CLANG_TIDY_EXE} -quiet -p ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy analysis"
        VERBATIM
    )
elseif(CLANG_TIDY_EXE)
    add_custom_target(clang-tidy
        COMMAND ${CLANG_TIDY_EXE} --quiet -p ${CMAKE_BINARY_DIR} ${ART2IMG_ALL_TRANSLATION_UNITS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy analysis"
        VERBATIM
    )
else()
    add_custom_target(clang-tidy
        COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy not found; install clang-tidy to enable linting support."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "clang-tidy not available"
    )
endif()

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
