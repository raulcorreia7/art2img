add_executable(test_image_utils
  test_image_utils.cpp
)

target_link_libraries(test_image_utils
  art2img
  doctest::doctest
)

target_include_directories(test_image_utils PRIVATE
  ${PROJECT_SOURCE_DIR}/tests
)

add_test(NAME test_image_utils COMMAND test_image_utils)