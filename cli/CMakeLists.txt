# CMakeLists.txt for CLI
# 
# CLI executable for art2img using the new pipeline

# ============================================================================
# 1. DEPENDENCIES
# ============================================================================

# Add CLI11 dependency
CPMAddPackage(
    NAME CLI11
    VERSION 2.5.0
    GITHUB_REPOSITORY "CLIUtils/CLI11"
    GIT_TAG "v2.5.0"
)

# Add fmt dependency
CPMAddPackage(
    NAME fmt
    VERSION 11.0.2
    GITHUB_REPOSITORY "fmtlib/fmt"
    GIT_TAG "11.0.2"
)

# ============================================================================
# 2. CLI EXECUTABLE
# ============================================================================

# Create the CLI executable
add_executable(art2img_cli main.cpp)

# Set C++ standard
set_target_properties(art2img_cli PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Link with the core library
target_link_libraries(art2img_cli 
    PRIVATE 
    art2img_core
    CLI11::CLI11
    fmt::fmt
)

# Add include directories
target_include_directories(art2img_cli 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# ============================================================================
# 3. INSTALLATION
# ============================================================================

# Install the CLI executable
install(TARGETS art2img_cli
    RUNTIME DESTINATION bin
    COMPONENT cli
)

# Create a symlink or copy with the expected name
if(UNIX AND NOT APPLE)
    # On Linux, create a symlink
    install(CODE "
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            art2img_cli
            \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/art2img
        )
    " COMPONENT cli)
elseif(WIN32)
    # On Windows, copy with different name
    install(TARGETS art2img_cli
        RUNTIME DESTINATION bin
        RENAME art2img.exe
        COMPONENT cli
    )
else()
    # On other systems, copy with different name
    install(TARGETS art2img_cli
        RUNTIME DESTINATION bin
        RENAME art2img
        COMPONENT cli
    )
endif()

message(STATUS "CLI configured successfully")