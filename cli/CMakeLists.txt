cmake_minimum_required(VERSION 3.14)
project(art2img_cli)

# Create the main executable
add_executable(art2img
    ../src/art2img.cpp
)

# Set target properties
set_target_properties(art2img
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add include directories
target_include_directories(art2img
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_BINARY_DIR}/../include
)

# Link the library
target_link_libraries(art2img
    PRIVATE
    art2img_extractor
    CLI11::CLI11
)

# Add pthread flags if needed
if(NOT MSVC)
    target_link_options(art2img PRIVATE -pthread)
endif()

# Install the CLI executable if requested
if(BUILD_CLI)
    install(TARGETS art2img
        DESTINATION bin
    )
endif()
