cmake_minimum_required(VERSION 3.14)
project(art2img VERSION 1.0.0)

# CMake options
option(BUILD_CLI "Build the command line interface tool" ON)
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler-specific flags
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Set the output directory for the library files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add vendor directory
include_directories(vendor)

# Include the source directory
include_directories(include)

# Create the extractor library
add_library(art2img_extractor
    src/extractor_api.cpp
    src/extractor.cpp
    src/art_file.cpp
    src/palette.cpp
    src/png_writer.cpp
    src/tga_writer.cpp
    src/exceptions.cpp
)

# Add header-only files
target_sources(art2img_extractor
    PRIVATE
    include/extractor_api.hpp
    include/extractor.hpp
    include/art_file.hpp
    include/palette.hpp
    include/png_writer.hpp
    include/tga_writer.hpp
    include/exceptions.hpp
    include/version.hpp
    vendor/stb/stb_image_write.h
)

# Add the library to the build if it is successful
set_target_properties(art2img_extractor
    PROPERTIES
    PUBLIC_HEADER "include/extractor_api.hpp;include/extractor.hpp;include/art_file.hpp;include/palette.hpp;include/png_writer.hpp;include/tga_writer.hpp;include/exceptions.hpp;include/version.hpp"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Include the extract directories
target_include_directories(art2img_extractor
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Include the cli tool
if (BUILD_CLI)
    add_subdirectory(cli)
endif()

# Add the tests
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()