# ============================================================================
# TEST SOURCES
# ============================================================================
file(GLOB_RECURSE TEST_SOURCES
    unit/*.cpp
    unit/*/*.cpp
    unit/*/*/*.cpp
    test_setup.cpp
)

# ============================================================================
# TEST EXECUTABLE
# ============================================================================
add_executable(art2img_tests ${TEST_SOURCES})

# ============================================================================
# TEST CONFIGURATION
# ============================================================================
target_link_libraries(art2img_tests PRIVATE libart2img doctest::doctest_with_main)

# Test configuration - CMake will handle directory creation automatically
target_compile_definitions(art2img_tests PRIVATE 
    TEST_ASSET_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    TEST_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/test_output"
)

# ============================================================================
# TEST DISCOVERY
# ============================================================================
include(${DOCTEST_CMAKE_HELPER})
doctest_discover_tests(art2img_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTIES TIMEOUT 300
)

# ============================================================================
# BENCHMARKS
# ============================================================================
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()