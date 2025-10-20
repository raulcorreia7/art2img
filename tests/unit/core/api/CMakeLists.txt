add_executable(test_convenience
  test_convenience.cpp
)

target_link_libraries(test_convenience
  art2img
  doctest::doctest
)

target_include_directories(test_convenience PRIVATE
  ${PROJECT_SOURCE_DIR}/tests
)

add_test(NAME test_convenience COMMAND test_convenience)