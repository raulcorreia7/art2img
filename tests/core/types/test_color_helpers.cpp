#include <doctest/doctest.h>

#include <array>
#include <art2img/color_helpers.hpp>
#include <art2img/types.hpp>

TEST_SUITE("types::color_helpers") {

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

  TEST_CASE("write_rgba/read_rgba use shared layout") {
    std::array<art2img::types::u8, art2img::constants::RGBA_BYTES_PER_PIXEL>
        buffer{};
    const auto original = art2img::color::make_rgba(5, 6, 7, 8);

    art2img::color::write_rgba(buffer.data(), original);
    const auto roundtrip = art2img::color::read_rgba(buffer.data());

    CHECK(roundtrip == original);
  }
}
