cmake_minimum_required(VERSION 3.14)
project(art2img_tests)

# Add the test executable
add_executable(art2img_test
    test_library_api.cpp
    simple_test.cpp
    test_blood_palette_validation.cpp
    png_memory_regression.cpp
)

# Set the properties of the test executable
target_link_libraries(art2img_test
    PRIVATE
    art2img_extractor
)

# Include the directories
target_include_directories(art2img_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add test directory
add_test(NAME art2img_test COMMAND art2img_test)