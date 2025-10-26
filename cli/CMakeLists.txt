# ============================================================================
# DEPENDENCIES
# ============================================================================
CPMAddPackage(
    NAME CLI11
    VERSION 2.5.0
    GITHUB_REPOSITORY "CLIUtils/CLI11"
    GIT_TAG "v2.5.0"
)

# ============================================================================
# CLI EXECUTABLE
# ============================================================================
add_executable(art2img main.cpp)

target_link_libraries(art2img 
    PRIVATE 
    libart2img
    CLI11::CLI11
)

target_include_directories(art2img 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# ============================================================================
# INSTALLATION
# ============================================================================
# Simplified installation using generator expressions for platform-specific naming
install(TARGETS art2img
    RUNTIME DESTINATION bin
)