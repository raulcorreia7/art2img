cmake_minimum_required(VERSION 3.20)

# ============================================================================
# PROJECT SETUP
# ============================================================================
project(art2img 
    VERSION 1.0.0
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

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    if(NOT WIN32)
        add_compile_options(-pthread)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# ============================================================================
# DEPENDENCIES
# ============================================================================
set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/.cache")
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG fede005abaf93d9d7f3a679d1999b2db341b360f
)

add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE ${stb_SOURCE_DIR})
set_target_properties(stb_headers PROPERTIES
    INTERFACE_COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:CXX>:-w>"
)
add_library(stb ALIAS stb_headers)

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
# LIBRARY CREATION
# ============================================================================
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS "include/art2img/*.hpp")

if(NOT ART2IMG_ENABLE_LEGACY)
    list(FILTER LIB_SOURCES EXCLUDE REGEX "legacy_api")
endif()

add_library(art2img_core ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(art2img_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(TARGET stb)
    target_link_libraries(art2img_core PUBLIC stb)
    target_compile_definitions(art2img_core PUBLIC STB_IMAGE_WRITE_IMPLEMENTATION)
endif()

# ============================================================================
# INSTALLATION
# ============================================================================
install(TARGETS art2img_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/art2img
    DESTINATION include
)

# ============================================================================
# LEGACY API
# ============================================================================
option(ART2IMG_ENABLE_LEGACY "Enable legacy API compatibility wrapper" OFF)

if(ART2IMG_ENABLE_LEGACY)
    file(GLOB LEGACY_SOURCES CONFIGURE_DEPENDS "src/legacy_api.cpp")
    file(GLOB LEGACY_HEADERS CONFIGURE_DEPENDS "include/art2img/legacy_api.hpp")
    
    if(LEGACY_SOURCES AND LEGACY_HEADERS)
        target_sources(art2img_core PRIVATE ${LEGACY_SOURCES})
        target_sources(art2img_core PUBLIC ${LEGACY_HEADERS})
        target_compile_definitions(art2img_core PUBLIC ART2IMG_ENABLE_LEGACY)
        message(STATUS "Legacy API wrapper enabled")
    else()
        message(WARNING "Legacy API sources not found - legacy wrapper disabled")
    endif()
endif()

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