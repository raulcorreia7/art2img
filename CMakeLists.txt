# CMakeLists.txt
# 
# Main CMake file for art2img project

# ============================================================================
# 1. PROJECT SETUP
# ============================================================================
cmake_minimum_required(VERSION 3.20)

project(art2img 
    VERSION 2.0.0
    DESCRIPTION "ART file to image converter library"
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/raulcorreia7/art2img"
)

# Prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

# ============================================================================
# 2. COMPILER CONFIGURATION
# ============================================================================
# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    if(NOT WIN32)
        add_compile_options(-pthread)
    endif()
endif()

# Release optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# ============================================================================
# 3. DEPENDENCIES
# ============================================================================
set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/.cache")
include(cmake/CPM.cmake)

# Add stb library for image encoding
CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG fede005abaf93d9d7f3a679d1999b2db341b360f  # Latest commit on master branch
)

# Create interface library for stb since CPM doesn't create it automatically
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE ${stb_SOURCE_DIR})
# Suppress warnings from stb headers
set_target_properties(stb_headers PROPERTIES
    INTERFACE_COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:CXX>:-w>"
)
add_library(stb ALIAS stb_headers)

# Enable testing support
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    # Add doctest for testing
    CPMAddPackage(
        NAME doctest
        VERSION 2.4.12
        GITHUB_REPOSITORY "doctest/doctest"
        GIT_TAG "v2.4.12"
    )
endif()

# ============================================================================
# 4. LIBRARY CREATION
# ============================================================================
# Auto-discover source and header files
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS "include/art2img/*.hpp")

# Remove legacy API files if not enabled
if(NOT ART2IMG_ENABLE_LEGACY)
    list(FILTER LIB_SOURCES EXCLUDE REGEX "legacy_api")
endif()

# Validate files exist
if(NOT LIB_SOURCES)
    message(WARNING "No source files found in src/ directory - library may be incomplete")
endif()

if(NOT LIB_HEADERS)
    message(WARNING "No header files found in include/art2img/ directory")
endif()

# Create the main library
add_library(art2img_core ${LIB_SOURCES} ${LIB_HEADERS})

# Set library properties
set_target_properties(art2img_core PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Set up include directories
target_include_directories(art2img_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link stb library if available
if(TARGET stb)
    target_link_libraries(art2img_core PUBLIC stb)
    # Add the STB implementation definition
    target_compile_definitions(art2img_core PUBLIC STB_IMAGE_WRITE_IMPLEMENTATION)
endif()

# ============================================================================
# 5. INSTALLATION
# ============================================================================
# Simple installation
install(TARGETS art2img_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/art2img
    DESTINATION include
)
# ============================================================================
# 6. LEGACY API
# ============================================================================
# Option to build legacy API wrapper (default OFF)
option(ART2IMG_ENABLE_LEGACY "Enable legacy API compatibility wrapper" OFF)

if(ART2IMG_ENABLE_LEGACY)
    # Add legacy API source files if enabled
    file(GLOB LEGACY_SOURCES CONFIGURE_DEPENDS "src/legacy_api.cpp")
    file(GLOB LEGACY_HEADERS CONFIGURE_DEPENDS "include/art2img/legacy_api.hpp")
    
    if(LEGACY_SOURCES AND LEGACY_HEADERS)
        target_sources(art2img_core PRIVATE ${LEGACY_SOURCES})
        target_sources(art2img_core PUBLIC ${LEGACY_HEADERS})
        
        # Add compile definition to indicate legacy support is enabled
        target_compile_definitions(art2img_core PUBLIC ART2IMG_ENABLE_LEGACY)
        
        message(STATUS "Legacy API wrapper enabled")
    else()
        message(WARNING "Legacy API sources not found - legacy wrapper disabled")
    endif()
endif()


# ============================================================================
# 7. CLI
# ============================================================================
# Option to build CLI
option(BUILD_CLI "Build CLI executable" ON)

if(BUILD_CLI)
    add_subdirectory(cli)
endif()

# ============================================================================
# 8. TESTING
# ============================================================================
# Add tests directory if testing is enabled
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

message(STATUS "art2img project configured - core library ready")