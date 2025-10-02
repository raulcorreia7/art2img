cmake_minimum_required(VERSION 3.14)
project(art2img_cli)

# Create the main executable
add_executable(art2img
    ../src/art2img.cpp
)

# Add include directories
target_include_directories(art2img
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# Link the library
target_link_libraries(art2img
    PRIVATE
    art2img_extractor
)

# Compiler-specific flags
target_compile_definitions(art2img
    PRIVATE
    ART2IMG_VERSION="1.0"
)

# Set the output directory
set_target_properties(art2img
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)