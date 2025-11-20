#include <art2img/grp.hpp>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <vector>

namespace art2img {

namespace {
constexpr std::string_view kSignature = "KenSilverman";
constexpr std::size_t kDirectoryEntrySize = 16;
constexpr std::size_t kNameSize = 12;

std::uint32_t read_u32(ByteSpan data, std::size_t offset)
{
  std::uint32_t value = 0;
  for (std::size_t i = 0; i < 4; ++i) {
    value |= static_cast<std::uint32_t>(std::to_integer<uint8_t>(data[offset + i])) << (8 * i);
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

} // namespace

Result<GrpFile> parse_grp(ByteSpan blob)
{
  if (blob.size() < kSignature.size() + 4) {
    return std::unexpected(Error{"file too small for GRP header"});
  }

  if (std::string_view(reinterpret_cast<const char*>(blob.data()),
                       kSignature.size()) != kSignature) {
    return std::unexpected(Error{"invalid GRP signature"});
  }

  const std::uint32_t entry_count = read_u32(blob, kSignature.size());
  const std::size_t directory_bytes =
      static_cast<std::size_t>(entry_count) * kDirectoryEntrySize;
  const std::size_t directory_offset = kSignature.size() + 4;

  if (blob.size() < directory_offset + directory_bytes) {
    return std::unexpected(Error{"GRP directory truncated"});
  }

  std::vector<GrpEntry> entries;
  entries.reserve(entry_count);
  std::size_t data_offset = directory_offset + directory_bytes;
  std::size_t directory_cursor = directory_offset;

  for (std::uint32_t i = 0; i < entry_count; ++i) {
    std::string name(
        reinterpret_cast<const char*>(blob.data() + directory_cursor),
        kNameSize);
    const auto normalised = normalise_name(name);
    const std::uint32_t size = read_u32(blob, directory_cursor + kNameSize);

    if (data_offset + size > blob.size()) {
      return std::unexpected(Error{"GRP entry exceeds file size"});
    }

    std::vector<Byte> entry_data(blob.begin() + data_offset,
                                 blob.begin() + data_offset + size);
    entries.push_back(GrpEntry{normalised, std::move(entry_data)});

    data_offset += size;
    directory_cursor += kDirectoryEntrySize;
  }

  return GrpFile{std::move(entries)};
}

} // namespace art2img
