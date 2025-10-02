cmake_minimum_required(VERSION 3.14)
project(art2img_tests)

# Initialize test dependencies list
set(TEST_DEPENDENCIES)

# Add separate test executables (each test file has its own main())
foreach(test_file IN ITEMS test_library_api simple_test test_blood_palette_validation png_memory_regression)
    add_executable(${test_file}
        ${test_file}.cpp
    )

    set_target_properties(${test_file}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    target_link_libraries(${test_file}
        PRIVATE
        art2img_extractor
    )

    target_include_directories(${test_file}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../vendor
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/../include
    )

    if(NOT MSVC)
        target_link_options(${test_file} PRIVATE -pthread)
    endif()

    # Add to CTest
    add_test(NAME ${test_file} COMMAND ${test_file})
    set_tests_properties(${test_file} PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    # Add to test dependencies list
    list(APPEND TEST_DEPENDENCIES ${test_file})
endforeach()

# Create custom target to copy test assets
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/tests/assets
        ${CMAKE_BINARY_DIR}/tests/assets
    COMMENT "Copying test assets"
)

# Add functionality test via shell script
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
    DEPENDS art2img ${TEST_DEPENDENCIES} copy_assets
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