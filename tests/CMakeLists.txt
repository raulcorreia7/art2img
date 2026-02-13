# tests/CMakeLists.txt - Modular test configuration
# ============================================================================
# This file defines separate test targets for unit, integration, and e2e tests.
# ============================================================================

# ----------------------------------------------------------------------------
# Test Configuration Options
# ----------------------------------------------------------------------------
option(TEST_SKIP_SHAREWARE "Skip tests that require shareware GRP" OFF)
option(TEST_BUILD_UNIT "Build unit tests" ON)
option(TEST_BUILD_INTEGRATION "Build integration tests" ON)
option(TEST_BUILD_E2E "Build end-to-end tests" ON)

# ----------------------------------------------------------------------------
# Test Dependencies
# ----------------------------------------------------------------------------
# All test targets need doctest from tests/include/
set(TEST_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor
)

# ----------------------------------------------------------------------------
# Shared Test Infrastructure
# ----------------------------------------------------------------------------
# main.cpp contains the doctest main() and shared test utilities
set(TEST_MAIN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# Test harness provides common utilities and fixtures
set(TEST_HARNESS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_harness.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_harness.hpp
)

# ----------------------------------------------------------------------------
# Unit Tests (art2img-test-unit)
# ----------------------------------------------------------------------------
# Fast, no external dependencies. Tests core library functionality with mocks.
if(TEST_BUILD_UNIT)
    file(GLOB_RECURSE TEST_UNIT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp)

    add_executable(art2img-test-unit
        ${TEST_MAIN_SOURCE}
        ${TEST_HARNESS_SOURCES}
        ${TEST_UNIT_SOURCES}
    )

    target_include_directories(art2img-test-unit PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(art2img-test-unit PRIVATE art2img)

    # Define UNIT_TEST to allow tests to detect target type if needed
    target_compile_definitions(art2img-test-unit PRIVATE UNIT_TEST)

    add_test(NAME unit COMMAND art2img-test-unit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(unit PROPERTIES LABELS "unit")
endif()

# ----------------------------------------------------------------------------
# Integration Tests (art2img-test-integration)
# ----------------------------------------------------------------------------
# Tests that require shareware GRP file for realistic data testing.
# These tests are skipped at runtime if shareware is not available.
if(TEST_BUILD_INTEGRATION)
    file(GLOB_RECURSE TEST_INTEGRATION_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp)

    if(TEST_SKIP_SHAREWARE)
        # Build but skip shareware tests via compile definition
        add_executable(art2img-test-integration
            ${TEST_MAIN_SOURCE}
            ${TEST_HARNESS_SOURCES}
            ${TEST_INTEGRATION_SOURCES}
        )
        target_compile_definitions(art2img-test-integration PRIVATE SKIP_SHAREWARE_TESTS)
    else()
        add_executable(art2img-test-integration
            ${TEST_MAIN_SOURCE}
            ${TEST_HARNESS_SOURCES}
            ${TEST_INTEGRATION_SOURCES}
        )
    endif()

    target_include_directories(art2img-test-integration PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(art2img-test-integration PRIVATE art2img)

    target_compile_definitions(art2img-test-integration PRIVATE INTEGRATION_TEST)

    add_test(NAME integration COMMAND art2img-test-integration WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(integration PROPERTIES LABELS "integration")
endif()

# ----------------------------------------------------------------------------
# End-to-End Tests (art2img-test-e2e)
# ----------------------------------------------------------------------------
# Tests the CLI binary. Requires the art2img-cli executable to be built.
if(TEST_BUILD_E2E)
    file(GLOB_RECURSE TEST_E2E_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/e2e/*.cpp)

    if(TEST_SKIP_SHAREWARE)
        add_executable(art2img-test-e2e
            ${TEST_MAIN_SOURCE}
            ${TEST_HARNESS_SOURCES}
            ${TEST_E2E_SOURCES}
        )
        target_compile_definitions(art2img-test-e2e PRIVATE SKIP_SHAREWARE_TESTS)
    else()
        add_executable(art2img-test-e2e
            ${TEST_MAIN_SOURCE}
            ${TEST_HARNESS_SOURCES}
            ${TEST_E2E_SOURCES}
        )
    endif()

    target_include_directories(art2img-test-e2e PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(art2img-test-e2e PRIVATE art2img)

    # Pass CLI binary path as compile definition
    # E2E tests use this to locate the CLI executable
    target_compile_definitions(art2img-test-e2e PRIVATE
        E2E_TEST
        CLI_BINARY_PATH="$<TARGET_FILE:art2img-cli>"
    )

    # Ensure CLI is built before E2E tests
    add_dependencies(art2img-test-e2e art2img-cli)

    add_test(NAME e2e COMMAND art2img-test-e2e WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(e2e PROPERTIES LABELS "e2e")
endif()

# ----------------------------------------------------------------------------
# Legacy All-in-One Test Target (art2img-test)
# ----------------------------------------------------------------------------
# For backward compatibility. Builds all tests in a single executable.
if(NOT DEFINED TEST_BUILD_LEGACY OR TEST_BUILD_LEGACY)
    file(GLOB_RECURSE TEST_ALL_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/e2e/*.cpp
    )

    add_executable(art2img-test
        ${TEST_MAIN_SOURCE}
        ${TEST_HARNESS_SOURCES}
        ${TEST_ALL_SOURCES}
    )

    target_include_directories(art2img-test PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(art2img-test PRIVATE art2img)

    if(TEST_SKIP_SHAREWARE)
        target_compile_definitions(art2img-test PRIVATE SKIP_SHAREWARE_TESTS)
    endif()

    # Pass CLI binary path for E2E tests
    target_compile_definitions(art2img-test PRIVATE
        CLI_BINARY_PATH="$<TARGET_FILE:art2img-cli>"
    )

    add_dependencies(art2img-test art2img-cli)

    add_test(NAME art2img COMMAND art2img-test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(art2img PROPERTIES LABELS "all")
endif()

# ----------------------------------------------------------------------------
# Test Summary
# ----------------------------------------------------------------------------
message(STATUS "Test configuration:")
message(STATUS "  Unit tests:        ${TEST_BUILD_UNIT}")
message(STATUS "  Integration tests: ${TEST_BUILD_INTEGRATION}")
message(STATUS "  E2E tests:         ${TEST_BUILD_E2E}")
message(STATUS "  Skip shareware:    ${TEST_SKIP_SHAREWARE}")
