cmake_minimum_required(VERSION 3.14)

# Create unified doctest test executable
add_executable(art2img_tests)

# Add doctest source files
target_sources(art2img_tests
    PRIVATE
    doctest_tests.cpp
    test_art_file.cpp
    test_palette.cpp
    test_extractor_api.cpp
    test_image_view.cpp
    test_integration.cpp
    test_functionality.cpp
)

# Set test executable properties
set_target_properties(art2img_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
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

# Add to CTest
add_test(NAME doctest_tests COMMAND art2img_tests)
set_tests_properties(doctest_tests
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
