# ============================================================================
# INTEGRATION TEST SOURCES
# ============================================================================
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES
    *.cpp
)

# Add test setup file from parent directory
list(APPEND INTEGRATION_TEST_SOURCES ../test_setup.cpp)

# ============================================================================
# INTEGRATION TEST EXECUTABLE
# ============================================================================
add_executable(art2img_integration_tests ${INTEGRATION_TEST_SOURCES})

# ============================================================================
# INTEGRATION TEST CONFIGURATION
# ============================================================================
target_link_libraries(art2img_integration_tests PRIVATE libart2img doctest::doctest_with_main)

# Test configuration
target_compile_definitions(art2img_integration_tests PRIVATE 
    TEST_CLI_EXECUTABLE="${TEST_CLI_EXECUTABLE}"
    TEST_OUTPUT_DIR="${TEST_OUTPUT_BASE}/integration"
    TEST_ASSETS_DIR="${TEST_ASSETS_DIR}"
)

# ============================================================================
# INTEGRATION TEST DISCOVERY
# ============================================================================
include(${DOCTEST_CMAKE_HELPER})
doctest_discover_tests(art2img_integration_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    PROPERTIES TIMEOUT 300
)