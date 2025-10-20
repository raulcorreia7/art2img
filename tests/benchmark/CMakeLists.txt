# ============================================================================
# BENCHMARK SOURCES
# ============================================================================
file(GLOB_RECURSE BENCHMARK_SOURCES convert/*.cpp)

# ============================================================================
# BENCHMARK EXECUTABLES
# ============================================================================
foreach(BENCHMARK_SOURCE ${BENCHMARK_SOURCES})
    get_filename_component(BENCHMARK_NAME ${BENCHMARK_SOURCE} NAME_WE)
    add_executable(${BENCHMARK_NAME} ${BENCHMARK_SOURCE})
    
    target_link_libraries(${BENCHMARK_NAME} PRIVATE art2img_core)
    target_include_directories(${BENCHMARK_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
    
    target_compile_definitions(${BENCHMARK_NAME} PRIVATE 
        TEST_ASSET_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../assets"
    )
    
    set_target_properties(${BENCHMARK_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmarks
    )
endforeach()