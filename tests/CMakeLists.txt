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

if(NOT MSVC)
    target_link_options(art2img_tests PRIVATE -pthread)
endif()

# Add to CTest
add_test(NAME doctest_tests COMMAND art2img_tests)
set_tests_properties(doctest_tests
    PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Create custom target to copy test assets
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/tests/assets
        ${CMAKE_BINARY_DIR}/tests/assets
    COMMENT "Copying test assets"
)

# Add functionality test via shell script (keep existing shell tests)
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/test_functionality.sh)
    add_test(NAME functionality_tests
        COMMAND ${CMAKE_SOURCE_DIR}/tests/test_functionality.sh
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Add palette comparison test
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/test_palette_comparison.sh)
    add_test(NAME palette_comparison_tests
        COMMAND ${CMAKE_SOURCE_DIR}/tests/test_palette_comparison.sh
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Add palette functionality test
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/test_palette_functionality.sh)
    add_test(NAME palette_functionality_tests
        COMMAND ${CMAKE_SOURCE_DIR}/tests/test_palette_functionality.sh
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Add default palette validation test
if(EXISTS ${CMAKE_SOURCE_DIR}/tests/test_default_palette_validation.sh)
    add_test(NAME default_palette_validation_tests
        COMMAND ${CMAKE_SOURCE_DIR}/tests/test_default_palette_validation.sh
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Create a comprehensive test target that handles dependencies
add_custom_target(run_all_tests
    DEPENDS art2img art2img_tests copy_assets
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