#include <art2img/adapters/grp.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <expected>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace art2img::adapters {

namespace {
constexpr std::string_view kSignature = "KenSilverman";
constexpr std::size_t kDirectoryEntrySize = 16;
constexpr std::size_t kNameSize = 12;

std::uint32_t read_u32(std::span<const std::byte> data, std::size_t offset)
{
  std::uint32_t value = 0;
  for (std::size_t i = 0; i < 4; ++i) {
    value |= static_cast<std::uint32_t>(
                 std::to_integer<unsigned char>(data[offset + i]))
             << (8 * i);
  }
  return value;
}

std::string normalise_name(std::string_view name)
{
  std::string lowered(name);
  while (!lowered.empty() &&
         (lowered.back() == '\0' || lowered.back() == ' ')) {
    lowered.pop_back();
  }
  std::transform(
      lowered.begin(), lowered.end(), lowered.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return lowered;
}

}  // namespace

std::expected<GrpFile, core::Error> load_grp(
    std::span<const std::byte> blob) noexcept
{
  if (blob.size() < kSignature.size() + 4) {
    return std::unexpected(core::make_error(core::errc::invalid_art,
                                            "blob too small for GRP header"));
  }

  if (std::string_view(reinterpret_cast<const char*>(blob.data()),
                       kSignature.size()) != kSignature) {
    return std::unexpected(
        core::make_error(core::errc::invalid_art, "invalid GRP signature"));
  }

  const std::uint32_t entry_count = read_u32(blob, kSignature.size());
  const std::size_t directory_bytes =
      static_cast<std::size_t>(entry_count) * kDirectoryEntrySize;
  const std::size_t directory_offset = kSignature.size() + 4;

  if (blob.size() < directory_offset + directory_bytes) {
    return std::unexpected(
        core::make_error(core::errc::invalid_art, "GRP directory truncated"));
  }

  auto storage =
      std::make_shared<std::vector<std::byte>>(blob.begin(), blob.end());
  std::span<const std::byte> data(*storage);

  std::vector<GrpEntry> entries;
  entries.reserve(entry_count);
  std::size_t data_offset = directory_offset + directory_bytes;
  std::size_t directory_cursor = directory_offset;
  for (std::uint32_t i = 0; i < entry_count; ++i) {
    std::string name(
        reinterpret_cast<const char*>(data.data() + directory_cursor),
        kNameSize);
    const auto normalised = normalise_name(name);
    const std::uint32_t size = read_u32(data, directory_cursor + kNameSize);
    if (data_offset + size > data.size()) {
      return std::unexpected(core::make_error(core::errc::invalid_art,
                                              "GRP entry exceeds file size"));
    }
    entries.push_back(GrpEntry{normalised, data.subspan(data_offset, size)});
    data_offset += size;
    directory_cursor += kDirectoryEntrySize;
  }

  GrpFile file;
  file.entries_ = std::move(entries);
  file.storage_ = std::move(storage);
  return file;
}

std::optional<GrpEntry> GrpFile::entry(std::string_view name) const noexcept
{
  std::string key = normalise_name(name);
  for (const auto& entry : entries_) {
    if (entry.name == key) {
      return entry;
    }
  }
  return std::nullopt;
}

}  // namespace art2img::adapters
