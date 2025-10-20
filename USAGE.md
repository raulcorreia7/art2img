# art2img Usage

## Command Line Usage

```bash
# Convert a single ART file to PNG (default format)
art2img_cli TILES.ART

# Convert to specific format with output directory
art2img_cli TILES.ART --format tga --output output/

# Convert all ART files in a directory
art2img_cli art_folder/ --output images/

# Disable transparency fix (for some games)
art2img_cli TILES.ART --no-transparency-fix

# Extract animation data with custom INI filename
art2img_cli art_folder/ --export-animation --anim-ini-filename my_anim.ini --output game/
```

## Key Features

- **Multiple Formats**: Convert to PNG, TGA, or BMP
- **Transparency Support**: Automatic magenta transparency fixing
- **Animation Export**: Extract animation data with INI files
- **Batch Processing**: Convert entire directories of ART files
- **Custom Palettes**: Use custom palette files if needed
- **Cross-Platform**: Works on Windows, Linux, and macOS

## Command Line Options

```
art2img_cli [OPTIONS] <input>

Positionals:
  input TEXT:DIR REQUIRED     Input ART file or directory

Options:
  -h,--help                   Print this help message and exit
  -o,--output TEXT:DIR        Output directory (default: current directory)
  -f,--format ENUM:value in {png->0,tga->1,bmp->2} OR {0,1,2}
                              Output image format (default: png)
  -j,--jobs UINT              Number of parallel jobs (0 for auto-detect, default: 0)
  --no-transparency-fix       Disable Build Engine magenta transparency fixing
  --export-animation          Export animation data to INI file
  --anim-ini-filename TEXT    Animation INI filename (default: animation.ini)
  --no-anim-ini-references    Don't include image file references in animation INI
  -v,--verbose                Enable verbose output
  --version                   Display program version
```

## Library API Usage

The art2img library can also be used programmatically in C++ applications:

### Convenience Functions

```cpp
#include <art2img/api.hpp>

// Simple one-call conversion with auto-discovered palette
auto result = art2img::convert_and_export(
    "path/to/TILES.ART",
    "output/directory/",
    art2img::ImageFormat::png);

// Convert with explicit palette path
auto result = art2img::convert_and_export(
    "path/to/TILES.ART",
    "path/to/PALETTE.DAT",
    "output/directory/",
    art2img::ImageFormat::tga);

// Convert single tile to image in memory
auto image_result = art2img::convert_tile(
    "path/to/TILES.ART",
    "path/to/PALETTE.DAT",
    0);  // tile index
```

### Builder Patterns

For more complex configurations, use the builder patterns:

```cpp
#include <art2img/api.hpp>

// Configure conversion options using builder
auto conversion_options = art2img::ConversionOptionsBuilder()
    .fix_transparency(true)
    .apply_lookup(true)
    .shade_index(0)
    .premultiply_alpha(false)
    .matte_hygiene(true)
    .build();

// Configure export options using builder
auto export_options = art2img::ExportOptionsBuilder()
    .output_dir("output/")
    .format(art2img::ImageFormat::png)
    .organize_by_format(false)
    .filename_prefix("tile_")
    .conversion_options(conversion_options)  // Use the conversion options
    .enable_parallel(true)
    .max_threads(4)
    .build();

// Load and export
auto art_result = art2img::load_art_bundle("TILES.ART");
auto palette_result = art2img::load_palette("PALETTE.DAT");

if (art_result && palette_result) {
    auto export_result = art2img::export_art_bundle(
        art_result.value(),
        palette_result.value(),
        export_options);
}
```

### Animation Export Configuration

For animation data export, use the specialized builder:

```cpp
auto anim_config = art2img::AnimationExportConfigBuilder()
    .output_dir("anim_output/")
    .base_name("animation_tile")
    .include_non_animated(false)
    .generate_ini(true)
    .ini_filename("animation_data.ini")
    .image_format(art2img::ImageFormat::png)
    .include_image_references(true)
    .build();
```

## Requirements

- For proper conversion, place PALETTE.DAT in the same directory as your ART files
- The tool will automatically detect and use the palette file if available

## Library Integration

To use art2img as a library in your C++ project:

1. Include the library headers: `#include <art2img/api.hpp>`
2. Link against the `art2img_core` library
3. Ensure C++17 or later is used for compilation

The library is header-only for most functionality, with only the core conversion functions requiring linkage.