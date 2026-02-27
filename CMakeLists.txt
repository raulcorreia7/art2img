cmake_minimum_required(VERSION 3.10)
project(art2img LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------------------------------------------------------------------
# Library
# ----------------------------------------------------------------------------
add_library(art2img STATIC
    src/art2img.cpp
)

target_include_directories(art2img
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

target_compile_definitions(art2img PUBLIC ART2IMG_USE_STB_IMAGE)

# ----------------------------------------------------------------------------
# CLI Executable
# ----------------------------------------------------------------------------
add_executable(art2img-cli
    cli/main.cpp
    cli/args.cpp
    cli/ops.cpp
)

target_include_directories(art2img-cli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/cli
)

target_link_libraries(art2img-cli PRIVATE art2img)
set_target_properties(art2img-cli PROPERTIES OUTPUT_NAME art2img)

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
install(TARGETS art2img art2img-cli
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES include/art2img.hpp DESTINATION include)
