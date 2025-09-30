# art2image

Extract Duke3D ART files to TGA or PNG images with animation support.

## Quick Start

```bash
# Build
make

# Extract ART file to PNG
bin/art2image -f png tests/assets/TILES000.art

# Extract with 8 threads to output directory
bin/art2image -o ./output -t 8 -f png tests/assets/TILES000.art

# Process all ART files in directory
bin/art2image -f png -m tests/assets/
```

## Usage

```bash
art2image [OPTIONS] <ART_FILE|ART_DIRECTORY>

Options:
  -o, --output DIR     Output directory (default: current)
  -t, --threads N      Number of threads (default: CPU cores)
  -p, --palette FILE   Palette file path (default: auto-detect)
  -f, --format FMT     Output format: tga or png (default: tga)
  -q, --quiet          Suppress verbose output
  -n, --no-anim        Don't generate animdata.ini
  -m, --merge-anim     Merge animation data into single file
  -h, --help           Show this help message
  -v, --version        Show version information
```



## Building & Testing

```bash
make          # Build release version
make debug    # Build with debug symbols
make clean    # Clean build artifacts
make test     # Run basic tests
make test-all # Run all tests
```

## Features

- Multi-threaded ART file extraction
- TGA and PNG output formats
- Animation data extraction (animdata.ini)
- Palette support for Duke3D color mapping
- Cross-platform (Windows/Linux)