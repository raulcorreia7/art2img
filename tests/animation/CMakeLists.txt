# Animation export tests
# This file defines the test targets for animation export functionality

# Create test executable
add_executable(test_animation_export
    test_animation_export.cpp
)

# Link with doctest and required libraries
target_link_libraries(test_animation_export
    PRIVATE
    doctest::doctest
    art2img_lib
)

# Add test to CTest
add_test(NAME AnimationExportTests COMMAND test_animation_export)

# Set test properties
set_tests_properties(AnimationExportTests PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Group tests
set_target_properties(test_animation_export PROPERTIES
    FOLDER "Tests"
)