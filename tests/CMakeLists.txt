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