# tests/CMakeLists.txt
# 
# Test configuration for art2img project

# ============================================================================
# 1. TEST SOURCES
# ============================================================================
file(GLOB_RECURSE TEST_SOURCES
                            core/*.cpp
                            art/*.cpp
                            convert/*.cpp
                            encode/*.cpp
                            io/*.cpp
                            integration/*.cpp)

# Add legacy tests if enabled
if(ART2IMG_ENABLE_LEGACY)
    file(GLOB LEGACY_TEST_SOURCES CONFIGURE_DEPENDS legacy/*.cpp)
    if(LEGACY_TEST_SOURCES)
        list(APPEND TEST_SOURCES ${LEGACY_TEST_SOURCES})
    endif()
endif()



# ============================================================================
# 2. TEST EXECUTABLE
# ============================================================================
add_executable(art2img_tests ${TEST_SOURCES})

# ============================================================================
# 3. TEST LINKING
# ============================================================================
target_link_libraries(art2img_tests PRIVATE art2img_core doctest::doctest_with_main)

# ============================================================================
# 4. TEST CONFIGURATION
# ============================================================================
target_compile_definitions(art2img_tests PRIVATE TEST_ASSET_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

# ============================================================================
# 5. TEST DISCOVERY
# ============================================================================
include(${DOCTEST_CMAKE_HELPER})
doctest_discover_tests(art2img_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTIES TIMEOUT 300
)

# ============================================================================
# 6. BENCHMARKS
# ============================================================================
# Add benchmarks directory (conditionally built)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()


