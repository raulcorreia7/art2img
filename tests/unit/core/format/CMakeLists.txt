add_executable(test_format_utils
  test_format_utils.cpp
)

target_link_libraries(test_format_utils
  art2img
  doctest::doctest
)

target_include_directories(test_format_utils PRIVATE
  ${PROJECT_SOURCE_DIR}/tests
)

add_test(NAME test_format_utils COMMAND test_format_utils)