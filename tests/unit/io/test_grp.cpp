#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <doctest/doctest.h>

#include <art2img/adapters/grp.hpp>
#include <art2img/core/error.hpp>

namespace {

std::vector<std::byte> make_grp_blob(
    std::initializer_list<std::pair<std::string, std::vector<std::byte>>>
        entries) {
  constexpr std::string_view signature = "KenSilverman";
  std::vector<std::byte> blob;
  blob.reserve(signature.size() + 4 + entries.size() * 16);
  for (char c : signature) {
    blob.push_back(static_cast<std::byte>(c));
  }

  const auto entry_count = static_cast<std::uint32_t>(entries.size());
  for (int i = 0; i < 4; ++i) {
    blob.push_back(static_cast<std::byte>((entry_count >> (8 * i)) & 0xFF));
  }

  std::vector<std::byte> payload;
  for (const auto& [name, data] : entries) {
    std::array<char, 12> name_field{};
    const auto copy_len = std::min(name.size(), name_field.size());
    std::copy_n(name.data(), copy_len, name_field.data());
    blob.insert(
        blob.end(), reinterpret_cast<std::byte*>(name_field.data()),
        reinterpret_cast<std::byte*>(name_field.data() + name_field.size()));

    const auto size = static_cast<std::uint32_t>(data.size());
    for (int i = 0; i < 4; ++i) {
      blob.push_back(static_cast<std::byte>((size >> (8 * i)) & 0xFF));
    }
    payload.insert(payload.end(), data.begin(), data.end());
  }

  blob.insert(blob.end(), payload.begin(), payload.end());
  return blob;
}

}  // namespace

TEST_CASE("load_grp parses entries and keeps storage alive") {
  const auto blob =
      make_grp_blob({{"FIRSTART", {std::byte{0x01}, std::byte{0x02}}},
                     {"SECONDART", {std::byte{0xAA}}}});

  const auto grp = art2img::adapters::load_grp(blob);
  REQUIRE(grp);
  CHECK(grp->entries().size() == 2);
  CHECK(grp->entries()[0].name == "firstart");
  CHECK(grp->entries()[1].data.size() == 1);

  const auto first = grp->entry("FIRSTART");
  REQUIRE(first);
  CHECK(first->data.size() == 2);
  CHECK(std::to_integer<unsigned char>(first->data[0]) == 0x01);
  CHECK(std::to_integer<unsigned char>(first->data[1]) == 0x02);

  const auto missing = grp->entry("MISSING");
  CHECK(!missing);
}

TEST_CASE("load_grp rejects invalid headers") {
  std::vector<std::byte> blob = {std::byte{0x00}, std::byte{0x01}};
  const auto result = art2img::adapters::load_grp(blob);
  CHECK(!result);
  CHECK(result.error().code == art2img::core::errc::invalid_art);
}
