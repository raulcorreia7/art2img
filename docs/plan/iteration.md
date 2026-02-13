# Iteration Plan — v1.2.0 Release

## v1.2.0 Release Summary

### Changes
- Consolidated library into single header (`include/art2img.hpp`) and source (`src/art2img.cpp`)
- Renamed types to PascalCase per naming conventions
- Added parallel processing to CLI (`-j`, `--jobs`, `--no-parallel`)
- Added quiet mode (`-q`, `--quiet`)
- Fixed ART→PNG transpose (column-major to row-major)
- Added transparency fix tests
- Test output moved to `build/test_output/`

### Module Inventory

| Namespace | Responsibility | Key Types |
|-----------|----------------|-----------|
| `art2img` | ART parsing, palette loading, conversion, encoding, error handling | `GrpFile`, `ArtFile`, `Palette`, `Image`, `RenderOptions`, `ConvertOptions`, `Format`, `Error`, `Result` |

### Current API Surface

```cpp
// Types
struct GrpFile;
struct ArtFile;
struct Palette;
struct Image;
struct RenderOptions;
struct ConvertOptions;
enum class Format { png, tga, bmp };
struct Error;
template<typename T> using Result = std::expected<T, Error>;

// GRP operations
Result<GrpFile> GrpFile::load(const std::string& path);
std::vector<std::string> GrpFile::list() const;
size_t GrpFile::count() const;

// ART operations
Result<ArtFile> ArtFile::load(const std::string& path);
Result<ArtFile> ArtFile::from_grp(const GrpFile& grp, const std::string& art_name);
size_t ArtFile::tile_count() const;
bool ArtFile::get_tile_size(size_t index, uint16_t* w, uint16_t* h) const;
Result<Image> ArtFile::get_tile(size_t index) const;

// Palette operations
Result<Palette> Palette::load(const std::string& path);
Result<Palette> Palette::from_grp(const GrpFile& grp);
void Palette::get_color(uint8_t idx, uint8_t rgba[4]) const;
void Palette::get_shaded_color(uint8_t idx, uint8_t shade, uint8_t rgba[4]) const;

// Rendering
Result<Image> render(const Tile& tile, const Palette& pal, const RenderOptions& opts = {});

// Encoding
Result<std::vector<std::byte>> encode(const Image& img, Format fmt);

// High-level
Result<std::vector<Image>> extract_all(const ArtFile& art, const Palette& pal, const RenderOptions& opts = {});
Result<size_t> convert_art(const ArtFile& art, const Palette& pal, const ConvertOptions& opts);

// IO
Result<void> write_file(const std::string& path, const std::vector<std::byte>& data);
```

### CLI Options

```
Usage: art2img [options] [input] [palette]

Modes:
  --list                List GRP contents and exit
  -h, --help           Show this help
  --version            Show version

Input Options:
  --grp <file>         Load ART from GRP archive
  --art <name>         ART filename in GRP (default: auto-detect)
  -p, --palette <file> Palette file (default: auto-detect from GRP)

Output Options:
  -o, --output <dir>   Output directory (default: current)
  -f, --format <fmt>   Output format: png, tga, bmp (default: png)

Rendering Options:
  --shade <n>          Apply shade table index (0-31)
  --lookup             Enable lookup table remapping
  --no-transparency    Disable transparency fix (index 255 opaque)
  --premultiply        Enable alpha premultiplication
  --matte              Enable matte hygiene

Performance:
  -j, --jobs <n>      Number of parallel jobs (0=auto)
  --no-parallel        Disable parallel processing

Other:
  -v, --verbose        Verbose output
  -q, --quiet          Suppress non-error output
```

## Follow-ups

- Add animation export support
- Add more image format options (HDR, JPEG)
- Provide parallel processing example in documentation
