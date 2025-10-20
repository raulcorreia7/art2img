# ============================================================================
# DEPENDENCIES
# ============================================================================
CPMAddPackage(
    NAME CLI11
    VERSION 2.5.0
    GITHUB_REPOSITORY "CLIUtils/CLI11"
    GIT_TAG "v2.5.0"
)

CPMAddPackage(
    NAME fmt
    VERSION 11.0.2
    GITHUB_REPOSITORY "fmtlib/fmt"
    GIT_TAG "11.0.2"
)

# ============================================================================
# CLI EXECUTABLE
# ============================================================================
add_executable(art2img_cli main.cpp)

target_link_libraries(art2img_cli 
    PRIVATE 
    art2img_core
    CLI11::CLI11
    fmt::fmt
)

target_include_directories(art2img_cli 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# ============================================================================
# INSTALLATION
# ============================================================================
install(TARGETS art2img_cli
    RUNTIME DESTINATION bin
    COMPONENT cli
)

if(UNIX AND NOT APPLE)
    install(CODE "
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            art2img_cli
            \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/art2img
        )
    " COMPONENT cli)
elseif(WIN32)
    install(TARGETS art2img_cli
        RUNTIME DESTINATION bin
        RENAME art2img.exe
        COMPONENT cli
    )
else()
    install(TARGETS art2img_cli
        RUNTIME DESTINATION bin
        RENAME art2img
        COMPONENT cli
    )
endif()