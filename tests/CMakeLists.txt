set(TEST_OUTPUT_BASE "${CMAKE_BINARY_DIR}/test_output")
set(TEST_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(TEST_CLI_EXECUTABLE "$<TARGET_FILE:art2img-cli>")

# ============================================================================
# TEST SUBDIRECTORIES
# ============================================================================
add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(smoke)

# ============================================================================
# BENCHMARKS
# ============================================================================
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()