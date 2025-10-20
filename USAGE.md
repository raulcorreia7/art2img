# art2img Usage

## Basic Usage

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

## Requirements

- For proper conversion, place PALETTE.DAT in the same directory as your ART files
- The tool will automatically detect and use the palette file if available