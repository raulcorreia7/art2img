<div align="center">
  <h1>art2img v0.2.0</h1>
  <p><strong>Convert Duke Nukem 3D ART files to modern image formats</strong></p>
</div>

A professional tool for game modders to convert Duke Nukem 3D ART files to PNG, TGA, or BMP with transparency and animation support.

## Quick Start for Modders

After building (see below), run:

```bash
# Basic conversion (Linux/Mac)
./art2img tiles.art

# Basic conversion (Windows)
./art2img.exe tiles.art

# Convert to specific format with output directory
./art2img tiles.art -f tga -o output/

# Convert all ART files in a directory
./art2img art/ -o images/

# For games with transparency (Green Slime, etc.)
./art2img tiles.art -F  # Enable transparency fix

# Extract animation data
./art2img art/ -m -o game/  # Merge animation data
```

## Command-Line Options

```
art2img [OPTIONS] ART_FILE|ART_DIRECTORY

POSITIONALS:
  ART_FILE|ART_DIRECTORY TEXT REQUIRED
                              Input ART file or directory containing ART files 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -v,     --version           Display program version information and exit 
  -o,     --output TEXT [.]   Output directory for converted images 
  -p,     --palette FILE      Custom palette file (defaults to built-in Duke Nukem 3D palette) 
  -f,     --format TEXT:{tga,png,bmp} [png]  
                              Output format: tga, png, or bmp 
  -F,     --fix-transparency, --no-fix-transparency{false} 
                              Enable magenta transparency fix (default: enabled) 
  -q,     --quiet             Suppress all non-essential output 
  -n,     --no-anim           Skip animation data generation 
  -m,     --merge-anim        Merge all animation data into a single file (directory mode) 
          --parallel, --no-parallel{false} 
                              Enable parallel tile export (default: enabled) 
  -j,     --jobs UINT:NONNEGATIVE [0]  
                              Maximum number of worker threads to use (0 = auto) 

Examples: 
art2img tiles.art # Convert single ART file 
art2img tiles.art -f tga -o out/ # Convert to TGA with output dir 
art2img art/ -o images/ # Convert all ART files 
art2img tiles.art -p custom.pal # Use custom palette 
art2img tiles.art --no-fix-transparency # Disable transparency 
art2img art/ -m -o game/ # Merge animation data 

For modders: Use -F for transparency and -m for animation data.
```

## Building the Project

**Requirements:**
- C++20 compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.14+

**Build Commands:**
```bash
make all                    # Build release version for your platform
make build                  # Build for Linux x64
make debug                  # Build debug version for Linux x64
make mingw-windows          # Cross-compile for Windows x64 using MinGW
make mingw-windows-x86      # Cross-compile for Windows x86 using MinGW
make test                   # Run tests on Linux
make clean                  # Clean build directory
```

**Cross-compilation for Windows (from Linux):**
```bash
# Install MinGW cross-compilers
sudo apt-get install g++-mingw-w64-x86-64 g++-mingw-w64-i686

# Build for Windows
make mingw-windows          # x64 version
make mingw-windows-x86      # x86 version
```

## License

[GPL v2](LICENSE) - Free and open-source software.

## Credits

Based on original work by Mathieu Olivier and Kenneth Silverman.
Modern C++20 implementation by [Raúl Correia](https://github.com/raulcorreia7).