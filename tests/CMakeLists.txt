# ============================================================================
# TEST SOURCES
# ============================================================================
file(GLOB_RECURSE TEST_SOURCES
    unit/*.cpp
    unit/*/*.cpp
    unit/*/*/*.cpp
    integration/*.cpp
    integration/*/*.cpp
    test_setup.cpp
)



# ============================================================================
# TEST EXECUTABLE
# ============================================================================
add_executable(art2img_tests ${TEST_SOURCES})

# ============================================================================
# TEST CONFIGURATION
# ============================================================================
target_link_libraries(art2img_tests PRIVATE art2img_core doctest::doctest_with_main)

target_compile_definitions(art2img_tests PRIVATE 
    TEST_ASSET_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    TEST_OUTPUT_DIR="${CMAKE_BINARY_DIR}/tests_output"
)

# Clean and create test output directory
file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/tests_output)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests_output)

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