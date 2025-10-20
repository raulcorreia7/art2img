# ============================================================================
# UNIT TESTS
# ============================================================================
# Define unit test sources
file(GLOB_RECURSE UNIT_TEST_SOURCES
    unit/core/*.cpp
    unit/art/*.cpp
    unit/convert/*.cpp
    unit/encode/*.cpp
    unit/export/*.cpp
    unit/io/*.cpp
    unit/animation/*.cpp
    unit/test_setup.cpp
)

# Create unit test executable
add_executable(tests_unit ${UNIT_TEST_SOURCES})

# Configure unit test executable
target_link_libraries(tests_unit PRIVATE art2img_core doctest::doctest_with_main)

target_compile_definitions(tests_unit PRIVATE 
    TEST_ASSET_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    TEST_OUTPUT_DIR="${CMAKE_BINARY_DIR}/tests_output"
)

# Clean and create test output directory at build time
# This approach avoids race conditions by creating the directory at build time rather than test time
add_custom_target(create_test_output_dir ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/tests_output
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/tests_output
    COMMENT "Creating test output directory"
)
add_dependencies(tests_unit create_test_output_dir)
# Add dependency after tests_integration target is created

# ============================================================================
# INTEGRATION TESTS
# ============================================================================
# Define integration test sources
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES
    integration/*/*.cpp
)

# Create integration test executable
add_executable(tests_integration ${INTEGRATION_TEST_SOURCES})

# Configure integration test executable
target_link_libraries(tests_integration PRIVATE art2img_core doctest::doctest_with_main)

target_include_directories(tests_integration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/unit)

target_compile_definitions(tests_integration PRIVATE 
    TEST_ASSET_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    TEST_OUTPUT_DIR="${CMAKE_BINARY_DIR}/tests_output"
)

# Add dependency on test output directory creation
add_dependencies(tests_integration create_test_output_dir)

# ============================================================================
# UNIT TEST DISCOVERY
# ============================================================================
include(${DOCTEST_CMAKE_HELPER})
doctest_discover_tests(tests_unit
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTIES TIMEOUT 300
)

# ============================================================================
# INTEGRATION TEST DISCOVERY
# ============================================================================
# Integration tests run sequentially
add_test(NAME integration_tests COMMAND tests_integration)
set_tests_properties(integration_tests PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ============================================================================
# BENCHMARKS
# ============================================================================
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()