#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

// Test the art2img/x.hpp include pattern
#include <art2img/file_operations.hpp>
#include <art2img/image_processor.hpp>
#include <art2img/image_writer.hpp>
#include <art2img/palette.hpp>

#include "art2img/art_file.hpp"
#include "art2img/colors.hpp"
#include "art2img/exceptions.hpp"
#include "art2img/extractor_api.hpp"
#include "art2img/file_operations.hpp"
#include "art2img/image_processor.hpp"
#include "art2img/image_writer.hpp"
#include "art2img/palette.hpp"

TEST_CASE("art2img include pattern verification") {
  SUBCASE("Basic namespace access") {
    // Just verify that the namespaces exist and are accessible
    CHECK(true);  // If compilation succeeds, the includes work
  }

  SUBCASE("Type availability") {
    // Verify that key types are available
    art2img::ArtFile art_file;
    art2img::Palette palette;
    CHECK(true);  // If compilation succeeds, types are available
  }
}