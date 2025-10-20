add_executable(test_error_formatting
  test_error_formatting.cpp
)

target_link_libraries(test_error_formatting
  art2img
  doctest::doctest
)

target_include_directories(test_error_formatting PRIVATE
  ${PROJECT_SOURCE_DIR}/tests
)

add_test(NAME test_error_formatting COMMAND test_error_formatting)