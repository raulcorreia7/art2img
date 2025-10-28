/// @file test_setup.cpp
/// @brief Global test setup and teardown
///
/// This file contains global setup/teardown logic that runs before/after
/// all tests to ensure a clean test environment.

#include <doctest/doctest.h>
#include <filesystem>
#include <iostream>

#include "test_helpers.hpp"

// Simple test to verify test helpers are working
TEST_CASE("Test Setup - Verify Helpers" * doctest::test_suite("Global Setup"))
{
  const auto test_output_dir = test_helpers::get_test_output_dir();

  // Ensure the directory exists first
  test_helpers::ensure_test_output_dir(test_output_dir);

  // Verify it exists
  CHECK(std::filesystem::exists(test_output_dir));

  // Test creating a test-specific directory
  auto test_dir = test_helpers::get_unit_test_dir("setup", "verify");
  test_helpers::ensure_test_output_dir(test_dir);
  CHECK(std::filesystem::exists(test_dir));

  // // Clean up after ourselves
  // test_helpers::cleanup_test_output_dir(test_dir);
  // CHECK(!std::filesystem::exists(test_dir));
}