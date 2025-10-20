# ============================================================================
# BENCHMARK TESTS
# ============================================================================
# Define benchmark test sources
file(GLOB_RECURSE BENCHMARK_SOURCES convert/*.cpp)

# Create benchmark test executable
add_executable(tests_benchmark ${BENCHMARK_SOURCES})

# Configure benchmark test executable
target_link_libraries(tests_benchmark PRIVATE art2img_core benchmark)

target_compile_definitions(tests_benchmark PRIVATE 
    TEST_ASSET_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../assets"
)

# Set output directory
set_target_properties(tests_benchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmarks
)