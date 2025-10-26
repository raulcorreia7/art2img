# ============================================================================
# UNIT TEST SOURCES
# ============================================================================
file(GLOB_RECURSE UNIT_TEST_SOURCES
    **/*.cpp
)

# Add test setup file from parent directory
list(APPEND UNIT_TEST_SOURCES ../test_setup.cpp)

# ============================================================================
# UNIT TEST EXECUTABLE
# ============================================================================
add_executable(art2img_unit_tests ${UNIT_TEST_SOURCES})

# ============================================================================
# UNIT TEST CONFIGURATION
# ============================================================================
target_link_libraries(art2img_unit_tests PRIVATE libart2img doctest::doctest_with_main)

# Test configuration
target_compile_definitions(art2img_unit_tests PRIVATE 
    TEST_CLI_EXECUTABLE="${TEST_CLI_EXECUTABLE}"
    TEST_OUTPUT_DIR="${TEST_OUTPUT_BASE}/unit"
    TEST_ASSETS_DIR="${TEST_ASSETS_DIR}"
)

# ============================================================================
# UNIT TEST DISCOVERY
# ============================================================================
include(${DOCTEST_CMAKE_HELPER})
doctest_discover_tests(art2img_unit_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    PROPERTIES TIMEOUT 300
)