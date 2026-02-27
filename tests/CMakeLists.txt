# tests/CMakeLists.txt
#
# Modular test configuration for art2img.

option(TEST_SKIP_SHAREWARE "Skip tests that require shareware GRP" OFF)
option(TEST_BUILD_UNIT "Build unit tests" ON)
option(TEST_BUILD_INTEGRATION "Build integration tests" ON)
option(TEST_BUILD_E2E "Build end-to-end tests" ON)

set(TEST_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor
)

set(TEST_MAIN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# ----------------------------------------------------------------------------
# Unit Tests
# ----------------------------------------------------------------------------
if(TEST_BUILD_UNIT)
    file(GLOB_RECURSE TEST_UNIT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp)

    add_executable(art2img-test-unit
        ${TEST_MAIN_SOURCE}
        ${TEST_UNIT_SOURCES}
    )

    target_include_directories(art2img-test-unit PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(art2img-test-unit PRIVATE art2img)
    target_compile_definitions(art2img-test-unit PRIVATE UNIT_TEST)

    add_test(NAME unit COMMAND art2img-test-unit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(unit PROPERTIES LABELS "unit")
endif()

# ----------------------------------------------------------------------------
# Integration Tests
# ----------------------------------------------------------------------------
if(TEST_BUILD_INTEGRATION)
    file(GLOB_RECURSE TEST_INTEGRATION_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp)

    add_executable(art2img-test-integration
        ${TEST_MAIN_SOURCE}
        ${TEST_INTEGRATION_SOURCES}
    )

    if(TEST_SKIP_SHAREWARE)
        target_compile_definitions(art2img-test-integration PRIVATE SKIP_SHAREWARE_TESTS)
    endif()

    target_include_directories(art2img-test-integration PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(art2img-test-integration PRIVATE art2img)
    target_compile_definitions(art2img-test-integration PRIVATE INTEGRATION_TEST)

    add_test(NAME integration COMMAND art2img-test-integration WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(integration PROPERTIES LABELS "integration")
endif()

# ----------------------------------------------------------------------------
# E2E Tests
# ----------------------------------------------------------------------------
if(TEST_BUILD_E2E)
    file(GLOB_RECURSE TEST_E2E_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/e2e/*.cpp)

    add_executable(art2img-test-e2e
        ${TEST_MAIN_SOURCE}
        ${TEST_E2E_SOURCES}
    )

    if(TEST_SKIP_SHAREWARE)
        target_compile_definitions(art2img-test-e2e PRIVATE SKIP_SHAREWARE_TESTS)
    endif()

    target_include_directories(art2img-test-e2e PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(art2img-test-e2e PRIVATE art2img)
    target_compile_definitions(art2img-test-e2e PRIVATE
        E2E_TEST
        CLI_BINARY_PATH="$<TARGET_FILE:art2img-cli>"
    )

    add_dependencies(art2img-test-e2e art2img-cli)

    add_test(NAME e2e COMMAND art2img-test-e2e WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(e2e PROPERTIES LABELS "e2e")
endif()

# ----------------------------------------------------------------------------
# Legacy All-in-One Target
# ----------------------------------------------------------------------------
if(NOT DEFINED TEST_BUILD_LEGACY OR TEST_BUILD_LEGACY)
    file(GLOB_RECURSE TEST_ALL_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/e2e/*.cpp
    )

    add_executable(art2img-test
        ${TEST_MAIN_SOURCE}
        ${TEST_ALL_SOURCES}
    )

    target_include_directories(art2img-test PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(art2img-test PRIVATE art2img)

    if(TEST_SKIP_SHAREWARE)
        target_compile_definitions(art2img-test PRIVATE SKIP_SHAREWARE_TESTS)
    endif()

    target_compile_definitions(art2img-test PRIVATE
        CLI_BINARY_PATH="$<TARGET_FILE:art2img-cli>"
    )

    add_dependencies(art2img-test art2img-cli)

    add_test(NAME art2img COMMAND art2img-test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(art2img PROPERTIES LABELS "all")
endif()

message(STATUS "Test configuration:")
message(STATUS "  Unit tests:        ${TEST_BUILD_UNIT}")
message(STATUS "  Integration tests: ${TEST_BUILD_INTEGRATION}")
message(STATUS "  E2E tests:         ${TEST_BUILD_E2E}")
message(STATUS "  Skip shareware:    ${TEST_SKIP_SHAREWARE}")
