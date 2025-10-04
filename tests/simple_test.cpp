#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include "extractor_api.hpp"

TEST_CASE("ExtractorAPI - Basic instantiation") {
  // Test that extractor can be instantiated without errors
  art2img::ExtractorAPI extractor;

  // Basic check that the extractor is in a valid state
  CHECK_EQ(extractor.get_tile_count(), 0);  // Should be 0 since no art file is loaded yet
}