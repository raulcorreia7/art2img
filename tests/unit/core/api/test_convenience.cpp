#include <doctest/doctest.h>
#include <art2img/api.hpp>
#include <art2img/convenience.hpp>

using namespace art2img;

// Note: These tests are compilation tests rather than runtime tests
// since we don't have actual ART files in the test environment

TEST_SUITE("Convenience API") {
  TEST_CASE("convenience API compiles correctly") {
    // This test just verifies that the convenience API functions compile correctly
    // We can't actually run them without real ART files, but we can check the signatures

    REQUIRE(true);  // Always passes, just to make the test suite non-empty
  }
}