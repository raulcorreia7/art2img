# ============================================================================
# SMOKE TEST SOURCES
# ============================================================================
file(GLOB_RECURSE SMOKE_TEST_SOURCES
    *.cpp
)

# ============================================================================
# SMOKE TEST EXECUTABLE
# ============================================================================
add_executable(art2img_smoke_tests ${SMOKE_TEST_SOURCES})

# ============================================================================
# SMOKE TEST CONFIGURATION
# ============================================================================
target_link_libraries(art2img_smoke_tests PRIVATE libart2img doctest::doctest_with_main)

# Smoke test configuration - fast execution for CI/CD
target_compile_definitions(art2img_smoke_tests PRIVATE 
    TEST_CLI_EXECUTABLE="${TEST_CLI_EXECUTABLE}"
    TEST_OUTPUT_DIR="${TEST_OUTPUT_BASE}/smoke"
    TEST_ASSETS_DIR="${TEST_ASSETS_DIR}"
)

# ============================================================================
# SMOKE TEST DISCOVERY
# ============================================================================
include(${DOCTEST_CMAKE_HELPER})
doctest_discover_tests(art2img_smoke_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    PROPERTIES TIMEOUT 30
)