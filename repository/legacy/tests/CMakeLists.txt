cmake_minimum_required(VERSION 3.14)

# Create unified doctest test executable
add_executable(art2img_tests)

# Add doctest source files - streamlined test suite
target_sources(art2img_tests
    PRIVATE
    doctest_tests.cpp
    ../cli/cli_app_builder.cpp
    ../cli/processor.cpp
    ../cli/config.cpp
    core/art_file/test_art_file.cpp
    core/art_file/test_image_view.cpp
    core/palette/test_palette.cpp
    core/palette/test_blood_palette_validation.cpp
    core/image_processor/test_transparency.cpp
    core/image_processor/test_transparency_files.cpp
    api/extractor_api/test_extractor_api.cpp
    api/extractor_api/test_batch_extraction_files.cpp
    api/extractor_api/test_full_extraction.cpp
    api/library_api/test_library_api.cpp
    cli/options/test_cli_options.cpp
    cli/options/test_cli_error_messages.cpp
    cli/processing/test_cli_processing_order.cpp
    cli/processing/test_output_directories.cpp
    cli/functionality/test_functionality.cpp
    format/bmp/test_bmp_format.cpp
    core/image_processor/test_png_memory_regression.cpp
    core/art_file/test_art2img_includes.cpp
    core/art_file/test_art2img_api.cpp
    integration/test_integration.cpp
    corruption/test_corrupted_files.cpp
)

# Set test executable properties
set_target_properties(art2img_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Set C++ standard via compile features
target_compile_features(art2img_tests
    PRIVATE
        cxx_std_20
)

# Link libraries
target_link_libraries(art2img_tests
    PRIVATE
    art2img_extractor
    doctest::doctest_with_main
    CLI11::CLI11
)

target_include_directories(art2img_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../cli
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/../include
)

set(TEST_ASSET_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tests/assets")
set(TEST_ASSET_BINARY_DIR "${CMAKE_BINARY_DIR}/tests/assets")

target_compile_definitions(art2img_tests
    PRIVATE
    TEST_ASSET_SOURCE_DIR="${TEST_ASSET_SOURCE_DIR}"
    TEST_ASSET_BINARY_DIR="${TEST_ASSET_BINARY_DIR}"
)

if(NOT MSVC)
    target_link_options(art2img_tests PRIVATE -pthread)
endif()

# Standalone test executables removed to streamline test suite

# Use doctest test discovery to automatically register individual test cases as CTest tests
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

# Check if we're cross-compiling for Windows
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # For Windows cross-compiled builds, disable automatic test discovery
    # Tests will be run manually via Wine in the build scripts
    message(STATUS "Cross-compiling for Windows - tests will be run manually with Wine")
else()
    # Native build or non-Windows cross-compilation - run tests directly
    doctest_discover_tests(art2img_tests
        TEST_PREFIX "doctest-"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        PROPERTIES 
            TIMEOUT 300
    )
endif()

# Standalone tests removed

add_custom_target(sync_test_assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_ASSET_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEST_ASSET_SOURCE_DIR} ${TEST_ASSET_BINARY_DIR}
    COMMENT "Syncing test assets"
)

add_dependencies(art2img_tests sync_test_assets)

# Create a comprehensive test target that handles dependencies
add_custom_target(run_all_tests
    DEPENDS art2img art2img_tests sync_test_assets
    COMMENT "Building test dependencies"
)

# Create test run command that depends on the build
add_custom_command(TARGET run_all_tests POST_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests"
)

# Create alias for convenience
add_custom_target(test_all
    DEPENDS run_all_tests
)
