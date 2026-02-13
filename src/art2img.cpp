#include "art2img.hpp"

#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace art2img {

namespace detail {
std::string format_string(const char* format, ...) {
  va_list args;
  va_start(args, format);

  va_list args_copy;
  va_copy(args_copy, args);
  int size = std::vsnprintf(nullptr, 0, format, args_copy);
  va_end(args_copy);

  if (size < 0)
    return "[format error]";

  std::string result(size, '\0');
  std::vsnprintf(&result[0], size + 1, format, args);
  va_end(args);

  return result;
}

inline uint32_t read_u32le(const std::byte* p) noexcept {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

inline uint16_t read_u16le(const std::byte* p) noexcept {
  return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

std::string normalize_filename(const std::string& name) {
  std::string r = name;
  while (!r.empty() && (r.back() == '\0' || r.back() == ' ')) {
    r.pop_back();
  }
  std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return r;
}

void apply_matte_hygiene(Image& img) noexcept {
  size_t w = img.width;
  size_t h = img.height;
  if (w < 3 || h < 3)
    return;

  std::vector<uint8_t> alpha(w * h);
  for (size_t i = 0; i < w * h; ++i) {
    alpha[i] = img.pixels[i * 4 + 3];
  }

  std::vector<uint8_t> eroded = alpha;
  for (size_t y = 1; y < h - 1; ++y) {
    for (size_t x = 1; x < w - 1; ++x) {
      size_t idx = y * w + x;
      if (alpha[idx] == 0)
        continue;
      uint8_t min_val = std::min({alpha[idx - w], alpha[idx + w], alpha[idx - 1], alpha[idx + 1]});
      eroded[idx] = min_val;
    }
  }

  for (size_t y = 1; y < h - 1; ++y) {
    for (size_t x = 1; x < w - 1; ++x) {
      size_t idx = y * w + x;
      uint32_t sum = 0;
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          sum += eroded[(y + dy) * w + (x + dx)];
        }
      }
      img.pixels[idx * 4 + 3] = static_cast<uint8_t>(sum / 9);
    }
  }
}

struct write_ctx {
  std::vector<std::byte>* data;
  bool failed = false;
};

void write_fn(void* c, void* d, int sz) {
  auto* p = static_cast<write_ctx*>(c);
  auto* b = static_cast<const std::byte*>(d);
  try {
    p->data->insert(p->data->end(), b, b + sz);
  } catch (...) {
    p->failed = true;
  }
}
}  // namespace detail

// ============================================================================
// Error
// ============================================================================

const char* to_string(error e) noexcept {
  switch (e) {
    case error::none:
      return "no error";
    case error::invalid_argument:
      return "invalid argument";
    case error::invalid_format:
      return "invalid file format";
    case error::corrupted_data:
      return "corrupted data";
    case error::io_error:
      return "io error";
    case error::not_found:
      return "not found";
    case error::out_of_memory:
      return "out of memory";
    case error::encoding_failed:
      return "encoding failed";
  }
  return "unknown error";
}

// ============================================================================
// GRP
// ============================================================================

class GrpFile::impl {
 public:
  std::vector<std::byte> data;
  struct entry {
    std::string name;
    size_t offset;
    size_t size;
  };
  std::vector<entry> entries;
};

GrpFile::GrpFile() noexcept = default;
GrpFile::~GrpFile() noexcept = default;
GrpFile::GrpFile(GrpFile&& other) noexcept = default;
GrpFile& GrpFile::operator=(GrpFile&& other) noexcept = default;

Result<GrpFile> GrpFile::load(const std::vector<std::byte>& data) noexcept {
  if (data.empty()) {
    return Error{error::invalid_argument, "GRP data is empty"};
  }

  if (data.size() < 16) {
    return Error{error::corrupted_data,
                 detail::format_string("GRP file too small: %zu bytes (minimum 16)", data.size())};
  }

  if (std::memcmp(data.data(), "KenSilverman", 12) != 0) {
    char sig[13] = {};
    std::memcpy(sig, data.data(), std::min(size_t(12), data.size()));
    return Error{
        error::invalid_format,
        detail::format_string("Invalid GRP signature: expected 'KenSilverman', got '%s'", sig)};
  }

  uint32_t num = detail::read_u32le(data.data() + 12);
  if (num > 100000) {
    return Error{error::corrupted_data,
                 detail::format_string("Invalid file count in GRP: %u (max 100000)", num)};
  }

  size_t dir_size = static_cast<size_t>(num) * 16;
  if (data.size() < 16 + dir_size) {
    return Error{error::corrupted_data,
                 detail::format_string("GRP directory truncated: need %zu bytes, have %zu",
                                       16 + dir_size, data.size())};
  }

  GrpFile result;
  try {
    result.pimpl_ = std::make_unique<impl>();
    result.pimpl_->data = data;
    result.pimpl_->entries.reserve(num);
  } catch (...) {
    return Error{error::out_of_memory,
                 detail::format_string("Failed to allocate GRP structure for %u files", num)};
  }

  size_t offset = 16 + dir_size;
  const std::byte* dir = data.data() + 16;

  for (uint32_t i = 0; i < num; ++i) {
    char name_raw[13] = {};
    std::memcpy(name_raw, dir, 12);
    uint32_t sz = detail::read_u32le(dir + 12);

    if (offset + sz > data.size()) {
      return Error{error::corrupted_data,
                   detail::format_string("GRP entry %u ('%s') exceeds bounds: "
                                         "offset=%zu, size=%u, total=%zu",
                                         i, name_raw, offset, sz, data.size())};
    }

    try {
      result.pimpl_->entries.push_back({detail::normalize_filename(name_raw), offset, sz});
    } catch (...) {
      return Error{error::out_of_memory,
                   detail::format_string("Failed to allocate entry for '%s'", name_raw)};
    }
    offset += sz;
    dir += 16;
  }

  return result;
}

Result<GrpFile> GrpFile::load(const std::string& path) noexcept {
  auto data_result = read_file(path);
  if (!data_result.ok()) {
    return Error{
        data_result.code(),
        detail::format_string("%s (path: '%s')", data_result.message().c_str(), path.c_str())};
  }
  return load(data_result.value());
}

std::vector<std::byte> GrpFile::get(const std::string& name) const noexcept {
  if (!pimpl_)
    return {};
  auto key = detail::normalize_filename(name);
  for (const auto& e : pimpl_->entries) {
    if (e.name == key) {
      try {
        return std::vector<std::byte>(pimpl_->data.begin() + e.offset,
                                      pimpl_->data.begin() + e.offset + e.size);
      } catch (...) {
        return {};
      }
    }
  }
  return {};
}

bool GrpFile::has(const std::string& name) const noexcept {
  return !get(name).empty();
}

std::vector<std::string> GrpFile::list() const noexcept {
  if (!pimpl_)
    return {};
  std::vector<std::string> r;
  try {
    r.reserve(pimpl_->entries.size());
    for (const auto& e : pimpl_->entries) {
      r.push_back(e.name);
    }
  } catch (...) {
    // Return what we have
  }
  return r;
}

size_t GrpFile::count() const noexcept {
  return pimpl_ ? pimpl_->entries.size() : 0;
}

// ============================================================================
// ART
// ============================================================================

class ArtFile::impl {
 public:
  std::vector<std::byte> data;
  uint32_t tile_start = 0;
  std::vector<std::pair<uint16_t, uint16_t>> dims;
  std::vector<uint32_t> picanm;
  size_t pixel_offset = 0;
  size_t lookup_offset = 0;
};

ArtFile::ArtFile() noexcept = default;
ArtFile::~ArtFile() noexcept = default;
ArtFile::ArtFile(ArtFile&& other) noexcept = default;
ArtFile& ArtFile::operator=(ArtFile&& other) noexcept = default;

Result<ArtFile> ArtFile::load(const std::vector<std::byte>& data) noexcept {
  if (data.size() < 16) {
    return Error{error::corrupted_data,
                 detail::format_string("ART file too small: %zu bytes (minimum 16)", data.size())};
  }

  uint32_t version = detail::read_u32le(data.data());
  if (version != 1) {
    return Error{error::invalid_format,
                 detail::format_string("Unsupported ART version: %u (expected 1)", version)};
  }

  uint32_t num_tiles_field = detail::read_u32le(data.data() + 4);
  uint32_t start = detail::read_u32le(data.data() + 8);
  uint32_t end = detail::read_u32le(data.data() + 12);

  // Handle zero tiles case: num_tiles field is 0 indicates empty ART file
  if (num_tiles_field == 0) {
    ArtFile result;
    try {
      result.pimpl_ = std::make_unique<impl>();
      result.pimpl_->data = data;
      result.pimpl_->tile_start = start;
    } catch (...) {
      return Error{error::out_of_memory, "Failed to allocate empty ART structure"};
    }
    return result;
  }

  if (end < start) {
    return Error{error::corrupted_data,
                 detail::format_string("Invalid tile range: end (%u) < start (%u)", end, start)};
  }

  uint32_t count = end - start + 1;
  if (count > 10000) {
    return Error{error::corrupted_data,
                 detail::format_string("Suspicious tile count: %u (max 10000)", count)};
  }

  size_t header_size = 16 + static_cast<size_t>(count) * 8;
  if (data.size() < header_size) {
    return Error{error::corrupted_data,
                 detail::format_string("ART header truncated: need %zu bytes, have %zu",
                                       header_size, data.size())};
  }

  ArtFile result;
  try {
    result.pimpl_ = std::make_unique<impl>();
    result.pimpl_->data = data;
    result.pimpl_->tile_start = start;
    result.pimpl_->dims.reserve(count);
    result.pimpl_->picanm.reserve(count);
  } catch (...) {
    return Error{error::out_of_memory,
                 detail::format_string("Failed to allocate ART structure for %u tiles", count)};
  }

  const std::byte* p = data.data() + 16;

  // Read widths
  for (uint32_t i = 0; i < count; ++i) {
    result.pimpl_->dims.push_back({detail::read_u16le(p), 0});
    p += 2;
  }
  // Read heights
  for (uint32_t i = 0; i < count; ++i) {
    result.pimpl_->dims[i].second = detail::read_u16le(p);
    p += 2;
  }
  // Read picanm
  for (uint32_t i = 0; i < count; ++i) {
    result.pimpl_->picanm.push_back(detail::read_u32le(p));
    p += 4;
  }

  result.pimpl_->pixel_offset = p - data.data();

  size_t total_pixels = 0;
  for (const auto& d : result.pimpl_->dims) {
    total_pixels += static_cast<size_t>(d.first) * d.second;
  }

  if (result.pimpl_->pixel_offset + total_pixels > data.size()) {
    return Error{error::corrupted_data,
                 detail::format_string("ART pixel data truncated: need %zu bytes from offset "
                                       "%zu, have %zu",
                                       total_pixels, result.pimpl_->pixel_offset, data.size())};
  }

  // Calculate lookup table offset (if present)
  result.pimpl_->lookup_offset = result.pimpl_->pixel_offset + total_pixels;

  return result;
}

Result<ArtFile> ArtFile::load(const std::string& path) noexcept {
  auto data_result = read_file(path);
  if (!data_result.ok()) {
    return Error{
        data_result.code(),
        detail::format_string("%s (path: '%s')", data_result.message().c_str(), path.c_str())};
  }
  return load(data_result.value());
}

Result<ArtFile> ArtFile::from_grp(const GrpFile& grp, const std::string& name) noexcept {
  auto data = grp.get(name);
  if (data.empty()) {
    return Error{error::not_found,
                 detail::format_string("ART file not found in GRP: '%s'", name.c_str())};
  }
  auto result = load(data);
  if (!result.ok()) {
    return Error{result.code(), detail::format_string("%s (file: '%s' in GRP)",
                                                      result.message().c_str(), name.c_str())};
  }
  return result;
}

uint32_t ArtFile::tile_start() const noexcept {
  return pimpl_ ? pimpl_->tile_start : 0;
}

size_t ArtFile::tile_count() const noexcept {
  return pimpl_ ? pimpl_->dims.size() : 0;
}

std::optional<Tile> ArtFile::get_tile(size_t index) const noexcept {
  if (!pimpl_ || index >= pimpl_->dims.size()) {
    return std::nullopt;
  }

  Tile t;
  t.width = pimpl_->dims[index].first;
  t.height = pimpl_->dims[index].second;
  t.count = static_cast<size_t>(t.width) * t.height;

  // Calculate pixel offset
  size_t off = pimpl_->pixel_offset;
  for (size_t i = 0; i < index; ++i) {
    off += static_cast<size_t>(pimpl_->dims[i].first) * pimpl_->dims[i].second;
  }
  t.indices = pimpl_->data.data() + off;

  // Calculate lookup table (if present)
  size_t lookup_total = pimpl_->data.size() - pimpl_->lookup_offset;
  if (lookup_total > 0) {
    size_t lookup_off = pimpl_->lookup_offset + index * 256;
    if (lookup_off + 256 <= pimpl_->data.size()) {
      t.lookup = pimpl_->data.data() + lookup_off;
      t.lookup_size = 256;
    }
  }

  return t;
}

std::optional<TileAnim> ArtFile::get_anim(size_t index) const noexcept {
  if (!pimpl_ || index >= pimpl_->picanm.size()) {
    return std::nullopt;
  }

  uint32_t raw = pimpl_->picanm[index];
  TileAnim anim;
  anim.picanm = raw;
  anim.anim_frames = raw & 0x0F;
  anim.anim_speed = (raw >> 4) & 0x0F;
  anim.anim_offset = (raw >> 8) & 0xFF;
  anim.xflip = (raw >> 16) & 1;
  anim.yflip = (raw >> 17) & 1;
  return anim;
}

bool ArtFile::get_tile_size(size_t index, uint16_t* width, uint16_t* height) const noexcept {
  if (!pimpl_ || index >= pimpl_->dims.size() || !width || !height) {
    return false;
  }
  *width = pimpl_->dims[index].first;
  *height = pimpl_->dims[index].second;
  return true;
}

// ============================================================================
// Palette
// ============================================================================

Result<Palette> Palette::load(const std::vector<std::byte>& data) noexcept {
  if (data.size() < 768) {
    return Error{error::corrupted_data,
                 detail::format_string("Palette too small: %zu bytes (expected 768)", data.size())};
  }

  Palette result;
  for (int i = 0; i < 256; ++i) {
    uint8_t r = static_cast<uint8_t>(data[i * 3]);
    uint8_t g = static_cast<uint8_t>(data[i * 3 + 1]);
    uint8_t b = static_cast<uint8_t>(data[i * 3 + 2]);

    // Scale 0-63 to 0-255
    result.colors_[i * 4 + 0] = (r << 2) | (r >> 4);
    result.colors_[i * 4 + 1] = (g << 2) | (g >> 4);
    result.colors_[i * 4 + 2] = (b << 2) | (b >> 4);
    result.colors_[i * 4 + 3] = (i == 255) ? 0 : 255;
  }

  return result;
}

Result<Palette> Palette::load(const std::string& path) noexcept {
  auto data_result = read_file(path);
  if (!data_result.ok()) {
    return Error{
        data_result.code(),
        detail::format_string("%s (path: '%s')", data_result.message().c_str(), path.c_str())};
  }
  auto result = load(data_result.value());
  if (!result.ok()) {
    return Error{result.code(),
                 detail::format_string("%s (file: '%s')", result.message().c_str(), path.c_str())};
  }
  return result;
}

Result<Palette> Palette::from_grp(const GrpFile& grp) noexcept {
  auto data = grp.get("palette.dat");
  if (data.empty()) {
    return Error{error::not_found, "PALETTE.DAT not found in GRP"};
  }
  if (data.size() < 768) {
    return Error{error::corrupted_data,
                 detail::format_string("PALETTE.DAT in GRP is truncated: %zu bytes", data.size())};
  }
  return load(data);
}

Result<Palette> Palette::load_extended(const std::vector<std::byte>& palette_dat,
                                       const std::vector<std::byte>& tabledata,
                                       const std::vector<std::byte>& lookup_dat) noexcept {
  auto base = load(palette_dat);
  if (!base.ok())
    return base;

  Palette result = base.value();

  // Load shade tables from TABLEDATA
  if (tabledata.size() >= 256) {
    try {
      result.shade_tables_.reserve(tabledata.size());
      for (auto b : tabledata) {
        result.shade_tables_.push_back(static_cast<uint8_t>(b));
      }
      result.num_shades_ = result.shade_tables_.size() / 256;
    } catch (...) {
      return Error{error::out_of_memory, "Failed to allocate shade tables"};
    }
  }

  // LOOKUP.DAT would be used for translucent tables (not implemented for now)
  (void)lookup_dat;

  return result;
}

bool Palette::get_color(uint8_t idx, uint8_t* out_rgba) const noexcept {
  if (!out_rgba)
    return false;
  out_rgba[0] = colors_[idx * 4 + 0];
  out_rgba[1] = colors_[idx * 4 + 1];
  out_rgba[2] = colors_[idx * 4 + 2];
  out_rgba[3] = colors_[idx * 4 + 3];
  return true;
}

bool Palette::get_shaded_color(uint8_t color_idx,
                               uint8_t shade_idx,
                               uint8_t* out_rgba) const noexcept {
  if (!out_rgba || shade_tables_.empty())
    return false;
  if (shade_idx >= num_shades_)
    return get_color(color_idx, out_rgba);

  uint8_t mapped_idx = shade_tables_[shade_idx * 256 + color_idx];
  return get_color(mapped_idx, out_rgba);
}

size_t Palette::shade_count() const noexcept {
  return num_shades_;
}

// ============================================================================
// Render
// ============================================================================

Result<Image> render(const Tile& t, const Palette& pal, const RenderOptions& opts) noexcept {
  if (!t.indices) {
    return Error{error::invalid_argument, "Tile has null pixel data"};
  }
  if (t.width == 0 || t.height == 0) {
    return Error{error::invalid_argument,
                 detail::format_string("Tile has zero dimensions: %ux%u", t.width, t.height)};
  }
  if (t.count == 0 || t.count > 100000000) {
    return Error{error::corrupted_data,
                 detail::format_string("Tile has invalid pixel count: %zu", t.count)};
  }

  Image result;
  result.width = t.width;
  result.height = t.height;

  try {
    result.pixels.resize(t.count * 4);
  } catch (...) {
    return Error{error::out_of_memory,
                 detail::format_string("Failed to allocate %zu bytes for image", t.count * 4)};
  }

  for (uint16_t y = 0; y < t.height; ++y) {
    for (uint16_t x = 0; x < t.width; ++x) {
      size_t src_idx = static_cast<size_t>(x) * t.height + y;
      size_t dst_idx = static_cast<size_t>(y) * t.width + x;
      uint8_t idx = static_cast<uint8_t>(t.indices[src_idx]);

      if (opts.apply_lookup && t.lookup && src_idx < t.lookup_size) {
        idx = static_cast<uint8_t>(t.lookup[idx]);
      }

      uint8_t rgba[4];
      if (opts.shade > 0 && pal.shade_count() > 0) {
        uint8_t shade_idx = std::min(opts.shade, static_cast<uint8_t>(pal.shade_count() - 1));
        pal.get_shaded_color(idx, shade_idx, rgba);
      } else {
        pal.get_color(idx, rgba);
      }

      if (idx == 255) {
        if (opts.fix_transparency) {
          rgba[3] = 0;
        } else {
          rgba[3] = 255;
        }
      }

      if (opts.premultiply && rgba[3] < 255) {
        rgba[0] = (rgba[0] * rgba[3]) / 255;
        rgba[1] = (rgba[1] * rgba[3]) / 255;
        rgba[2] = (rgba[2] * rgba[3]) / 255;
      }

      result.pixels[dst_idx * 4 + 0] = rgba[0];
      result.pixels[dst_idx * 4 + 1] = rgba[1];
      result.pixels[dst_idx * 4 + 2] = rgba[2];
      result.pixels[dst_idx * 4 + 3] = rgba[3];
    }
  }

  // Apply matte hygiene if requested
  if (opts.matte) {
    detail::apply_matte_hygiene(result);
  }

  return result;
}

// ============================================================================
// Encode
// ============================================================================

Result<std::vector<std::byte>> encode(const Image& img, Format fmt) noexcept {
  if (img.pixels.empty()) {
    return Error{error::invalid_argument, "Image has no pixel data"};
  }
  if (img.width == 0 || img.height == 0) {
    return Error{error::invalid_argument,
                 detail::format_string("Invalid image dimensions: %ux%u", img.width, img.height)};
  }

  size_t expected = static_cast<size_t>(img.width) * img.height * 4;
  if (img.pixels.size() != expected) {
    return Error{error::corrupted_data,
                 detail::format_string("Pixel buffer size mismatch: expected %zu, got %zu",
                                       expected, img.pixels.size())};
  }

  std::vector<std::byte> result;
  try {
    result.reserve(expected);
  } catch (...) {
    return Error{error::out_of_memory, "Failed to allocate encoding buffer"};
  }

  detail::write_ctx c{&result, false};
  int ok = 0;
  const char* fmt_name = "";

  switch (fmt) {
    case Format::png:
      ok = stbi_write_png_to_func(detail::write_fn, &c, img.width, img.height, 4, img.pixels.data(),
                                  img.width * 4);
      fmt_name = "PNG";
      break;
    case Format::tga:
      ok =
          stbi_write_tga_to_func(detail::write_fn, &c, img.width, img.height, 4, img.pixels.data());
      fmt_name = "TGA";
      break;
    case Format::bmp:
      ok =
          stbi_write_bmp_to_func(detail::write_fn, &c, img.width, img.height, 4, img.pixels.data());
      fmt_name = "BMP";
      break;
  }

  if (!ok || c.failed) {
    return Error{error::encoding_failed, detail::format_string("%s encoding failed for %ux%u image",
                                                               fmt_name, img.width, img.height)};
  }

  return result;
}

// ============================================================================
// File I/O
// ============================================================================

Result<std::vector<std::byte>> read_file(const std::string& path) noexcept {
  std::ifstream f(path, std::ios::binary | std::ios::ate);
  if (!f) {
    return Error{error::io_error, detail::format_string("Failed to open file: '%s'", path.c_str())};
  }

  auto sz = f.tellg();
  if (sz < 0) {
    return Error{error::io_error, detail::format_string("Failed to get size: '%s'", path.c_str())};
  }

  f.seekg(0, std::ios::beg);

  std::vector<std::byte> data;
  try {
    data.resize(static_cast<size_t>(sz));
  } catch (...) {
    return Error{error::out_of_memory,
                 detail::format_string("Failed to allocate %ld bytes for '%s'",
                                       static_cast<long>(sz), path.c_str())};
  }

  if (!f.read(reinterpret_cast<char*>(data.data()), sz)) {
    return Error{error::io_error, detail::format_string("Failed to read file: '%s'", path.c_str())};
  }

  return data;
}

Result<void> write_file(const std::string& path, const std::vector<std::byte>& data) noexcept {
  return write_file(path, data.data(), data.size());
}

Result<void> write_file(const std::string& path, const std::byte* data, size_t size) noexcept {
  if (!data && size > 0) {
    return Error{error::invalid_argument, "Null data pointer with non-zero size"};
  }

  std::ofstream f(path, std::ios::binary | std::ios::trunc);
  if (!f) {
    return Error{error::io_error,
                 detail::format_string("Failed to create file: '%s'", path.c_str())};
  }

  if (size > 0 && !f.write(reinterpret_cast<const char*>(data), size)) {
    return Error{error::io_error,
                 detail::format_string("Failed to write %zu bytes to: '%s'", size, path.c_str())};
  }

  return Result<void>{};
}

// ============================================================================
// High-level API
// ============================================================================

Result<std::vector<Image>> extract_all(const ArtFile& art,
                                       const Palette& pal,
                                       const RenderOptions& opts) noexcept {
  size_t n = art.tile_count();
  if (n == 0) {
    return Error{error::invalid_argument, "ART file has no tiles"};
  }

  std::vector<Image> result;
  try {
    result.reserve(n);
  } catch (...) {
    return Error{error::out_of_memory,
                 detail::format_string("Failed to allocate space for %zu images", n)};
  }

  for (size_t i = 0; i < n; ++i) {
    auto img_result = extract_tile(art, pal, i, opts);
    if (!img_result.ok()) {
      return Error{img_result.code(), detail::format_string("Failed to extract tile %zu: %s", i,
                                                            img_result.message().c_str())};
    }
    try {
      result.push_back(std::move(img_result.value()));
    } catch (...) {
      return Error{error::out_of_memory, detail::format_string("Failed to store tile %zu", i)};
    }
  }

  return result;
}

Result<Image> extract_tile(const ArtFile& art,
                           const Palette& pal,
                           size_t index,
                           const RenderOptions& opts) noexcept {
  auto tile_opt = art.get_tile(index);
  if (!tile_opt) {
    return Error{
        error::invalid_argument,
        detail::format_string("Tile index %zu out of range (count: %zu)", index, art.tile_count())};
  }
  return render(*tile_opt, pal, opts);
}

Result<std::vector<Image>> extract_range(const ArtFile& art,
                                         const Palette& pal,
                                         size_t start,
                                         size_t end,
                                         const RenderOptions& opts) noexcept {
  size_t count = art.tile_count();
  if (start >= count) {
    return Error{error::invalid_argument,
                 detail::format_string("Start index %zu >= tile count %zu", start, count)};
  }
  if (end > count) {
    end = count;
  }
  if (start >= end) {
    return Error{error::invalid_argument,
                 detail::format_string("Invalid range: [%zu, %zu)", start, end)};
  }

  std::vector<Image> result;
  try {
    result.reserve(end - start);
  } catch (...) {
    return Error{error::out_of_memory, "Failed to allocate result vector"};
  }

  for (size_t i = start; i < end; ++i) {
    auto img_result = extract_tile(art, pal, i, opts);
    if (!img_result.ok()) {
      return Error{img_result.code(), img_result.message()};
    }
    try {
      result.push_back(std::move(img_result.value()));
    } catch (...) {
      return Error{error::out_of_memory, detail::format_string("Failed to store tile %zu", i)};
    }
  }

  return result;
}

Result<size_t> convert_art(const ArtFile& art,
                           const Palette& pal,
                           const ConvertOptions& opts) noexcept {
  size_t count = art.tile_count();
  if (count == 0) {
    return Error{error::invalid_argument, "ART file has no tiles"};
  }

  size_t success = 0;

  for (size_t i = 0; i < count; ++i) {
    // Skip empty tiles (0x0 dimensions)
    uint16_t w, h;
    if (!art.get_tile_size(i, &w, &h) || w == 0 || h == 0) {
      continue;
    }

    auto img_result = extract_tile(art, pal, i, opts.render);
    if (!img_result.ok()) {
      return Error{img_result.code(),
                   detail::format_string("Tile %zu: %s", i, img_result.message().c_str())};
    }

    auto enc_result = encode(img_result.value(), opts.fmt);
    if (!enc_result.ok()) {
      return Error{enc_result.code(),
                   detail::format_string("Tile %zu: %s", i, enc_result.message().c_str())};
    }

    std::string filename = opts.output_prefix + std::to_string(i) + opts.output_suffix;
    auto write_result = write_file(filename, enc_result.value());
    if (!write_result.ok()) {
      return Error{write_result.code(),
                   detail::format_string("Tile %zu: %s", i, write_result.message().c_str())};
    }

    ++success;
  }

  return success;
}

}  // namespace art2img
