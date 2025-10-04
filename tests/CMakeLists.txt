cmake_minimum_required(VERSION 3.14)

# Create unified doctest test executable
add_executable(art2img_tests)

# Add doctest source files (excluding test_png_memory_regression.cpp as it's a standalone test)
target_sources(art2img_tests
    PRIVATE
    doctest_tests.cpp
    simple_test.cpp
    test_art_file.cpp
    test_blood_palette_validation.cpp
    test_bmp_format.cpp
    test_corrupted_files.cpp
    test_extractor_api.cpp
    test_functionality.cpp
    test_image_view.cpp
    test_integration.cpp
    test_library_api.cpp
    test_palette.cpp
    # Note: test_png_memory_regression.cpp is excluded as it's likely a standalone test
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
)

target_include_directories(art2img_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
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

# Create standalone test executables
add_executable(png_memory_regression
    png_memory_regression.cpp
)

# Set C++ standard via compile features
target_compile_features(png_memory_regression
    PRIVATE
        cxx_std_20
)
target_link_libraries(png_memory_regression
    PRIVATE
    art2img_extractor
)
target_include_directories(png_memory_regression
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/../include
)
if(NOT MSVC)
    target_link_options(png_memory_regression PRIVATE -pthread)
endif()

# Use doctest test discovery to automatically register individual test cases as CTest tests
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

doctest_discover_tests(art2img_tests
    TEST_PREFIX "doctest-"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    PROPERTIES 
        TIMEOUT 300
)

add_test(NAME png_memory_regression_test COMMAND png_memory_regression)
set_tests_properties(png_memory_regression_test
    PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

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
