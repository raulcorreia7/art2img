# art2img

**Convert Build Engine ART files to modern image formats**

A command-line utility for converting Duke Nukem 3D and other Build Engine ART files to PNG, TGA, or BMP formats. Designed for game modders who need to extract and modify game assets.

## Quick Start

### Option 1: Download Pre-built Binaries (Recommended)
Pre-built binaries for Windows, Linux, and macOS are available on the [Releases page](https://github.com/raulcorreia7/art2img/releases).

### Option 2: Build from Source

**Requirements:** CMake and a C++ compiler

```bash
# Clone and build
make build

# The executable will be at: build/linux-x64/cli/art2img_cli
# Or for Windows: build/windows-x64/cli/art2img_cli.exe
```

## Basic Usage

```bash
# Convert a single ART file to PNG
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
-h, --help                    Print this help message and exit
-o, --output DIR              Output directory (default: current directory)
-p, --palette FILE            Palette file path (default: auto-detect)
-f, --format FORMAT           Output format: png, tga, bmp (default: png)
    --no-transparency-fix     Disable transparency fix
    --shade INT               Apply shade table index (-1 to disable)
    --no-lookup               Disable lookup table application
    --premultiply-alpha       Premultiply alpha channel
    --matte-hygiene           Apply matte hygiene to remove halo effects
    --no-parallel             Disable parallel processing
-j, --jobs N                  Number of parallel jobs (0 for auto-detect)
    --export-animation        Export animation data instead of individual tiles
    --anim-ini-filename FILE  INI filename for animation data (default: animdata.ini)
    --include-non-animated-tiles Include non-animated tiles in animation export
-q, --quiet                   Suppress non-error output
-v, --verbose                 Verbose output
    --version                 Display program version information
```

## Troubleshooting

**Transparency issues:** Transparency fix is enabled by default. Use `--no-transparency-fix` to disable if needed

**Animation export:** Use `--export-animation` to export animation data with INI files

**Custom game palette:** Use `--palette custom.pal` with your palette file

**Performance:** Use `--jobs N` to control parallel processing (0 for auto-detect)

## License

[GPL v2](LICENSE) - Free and open-source software.

## Credits

Based on original work by Mathieu Olivier and Kenneth Silverman.
Modern implementation by [Raúl Correia](https://github.com/raulcorreia7).