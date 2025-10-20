#include <array>

#include <doctest/doctest.h>

#include <art2img/color_helpers.hpp>
#include <art2img/types.hpp>

#if __has_include(<span>)
#include <span>
#endif

TEST_SUITE("types::color_helpers") {
  TEST_CASE("make_rgba creates correct color") {
    const auto color = art2img::color::make_rgba(10, 20, 30, 40);

    CHECK(color.r == 10);
    CHECK(color.g == 20);
    CHECK(color.b == 30);
    CHECK(color.a == 40);
  }

  TEST_CASE("make_rgba with default alpha") {
    const auto color = art2img::color::make_rgba(10, 20, 30);

    CHECK(color.r == 10);
    CHECK(color.g == 20);
    CHECK(color.b == 30);
    CHECK(color.a == 255);
  }

  TEST_CASE("pack/unpack roundtrip") {
    const auto packed = art2img::color::pack_rgba(10, 20, 30, 40);
    const auto unpacked = art2img::color::unpack_rgba(packed);

    CHECK(unpacked.r == 10);
    CHECK(unpacked.g == 20);
    CHECK(unpacked.b == 30);
    CHECK(unpacked.a == 40);
  }

  TEST_CASE("make_from_bgr reorders channels") {
    const auto color = art2img::color::make_from_bgr(1, 2, 3, 4);

    CHECK(color.r == 3);
    CHECK(color.g == 2);
    CHECK(color.b == 1);
    CHECK(color.a == 4);
  }

  TEST_CASE("make_from_bgr with default alpha") {
    const auto color = art2img::color::make_from_bgr(1, 2, 3);

    CHECK(color.r == 3);
    CHECK(color.g == 2);
    CHECK(color.b == 1);
    CHECK(color.a == 255);
  }

  TEST_CASE("write_rgba/read_rgba use shared layout") {
    std::array<art2img::types::u8, art2img::constants::RGBA_BYTES_PER_PIXEL>
        buffer{};
    const auto original = art2img::color::make_rgba(5, 6, 7, 8);

    art2img::color::write_rgba(buffer.data(), original);
    const auto roundtrip = art2img::color::read_rgba(buffer.data());

    CHECK(roundtrip == original);
  }

#if __has_include(<span>)
  TEST_CASE("write_rgba/read_rgba with span use shared layout") {
    std::array<art2img::types::u8, art2img::constants::RGBA_CHANNEL_COUNT>
        buffer{};
    const auto original = art2img::color::make_rgba(9, 10, 11, 12);

    std::span<art2img::types::u8, art2img::constants::RGBA_CHANNEL_COUNT>
        buffer_span(buffer);
    std::span<const art2img::types::u8, art2img::constants::RGBA_CHANNEL_COUNT>
        const_buffer_span(buffer);

    art2img::color::write_rgba(buffer_span, original);
    const auto roundtrip = art2img::color::read_rgba(const_buffer_span);

    CHECK(roundtrip == original);
  }
#endif

  TEST_CASE("is_build_engine_magenta detects exact magenta") {
    CHECK(art2img::color::is_build_engine_magenta(252, 0, 252));
  }

  TEST_CASE("is_build_engine_magenta detects within tolerance") {
    CHECK(art2img::color::is_build_engine_magenta(250, 0, 250));
    CHECK(art2img::color::is_build_engine_magenta(255, 0, 255));
    CHECK(art2img::color::is_build_engine_magenta(251, 2, 251));
  }

  TEST_CASE("is_build_engine_magenta rejects non-magenta colors") {
    CHECK_FALSE(art2img::color::is_build_engine_magenta(255, 0, 0));  // Red
    CHECK_FALSE(art2img::color::is_build_engine_magenta(0, 255, 0));  // Green
    CHECK_FALSE(art2img::color::is_build_engine_magenta(0, 0, 255));  // Blue
    CHECK_FALSE(art2img::color::is_build_engine_magenta(
        249, 0, 249));  // Below tolerance
    CHECK_FALSE(art2img::color::is_build_engine_magenta(
        252, 6, 252));  // Green above tolerance
  }
}
