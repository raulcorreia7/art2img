add_executable(test_validation
  test_validation.cpp
)

target_link_libraries(test_validation
  art2img
  doctest::doctest
)

target_include_directories(test_validation PRIVATE
  ${PROJECT_SOURCE_DIR}/tests
)

add_test(NAME test_validation COMMAND test_validation)