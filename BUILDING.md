# Building art2img

## Requirements

- **C++20 compiler** (GCC 10+, Clang 10+, MSVC 2019+)
- **CMake** 3.14+
- **pthread** (Linux only)

## Build Instructions

### Quick Build
```bash
# Clone and build
git clone https://github.com/raulcorreia7/art2img.git
cd art2img
make all
```

### Makefile Commands
```bash
make all                    # Build for Linux (default)
make build                  # Build for Linux x64
make debug                  # Build debug version for Linux x64
make mingw-windows          # Cross-compile for Windows x64 using MinGW
make mingw-windows-x86      # Cross-compile for Windows x86 using MinGW
make linux-release          # Release build + tests for Linux
make mingw-windows-release  # Release build for Windows x64 using MinGW
make mingw-windows-x86-release # Release build for Windows x86 using MinGW
make test                   # Run tests on Linux
make test-windows           # Test Windows x64 build (requires Wine)
make test-windows-x86       # Test Windows x86 build (requires Wine)
make clean                  # Clean build directory
make install                # Install to system
make doctor                 # Check host dependencies
make fmt                    # Format code using clang-format
make fmt-check              # Check code formatting without modifying files
make lint                   # Run clang-tidy analysis
make verify                 # Run comprehensive checks (build, test, lint, format)
make setup                  # Set up development environment
```

## Cross-Platform Building

### Windows (Cross-compilation from Linux)
```bash
# Install MinGW cross-compilers
sudo apt-get install g++-mingw-w64-x86-64 g++-mingw-w64-i686

# Build for Windows
make mingw-windows          # x64 version
make mingw-windows-x86      # x86 version
```

### Windows (Native - requires Windows system)
Native Windows builds (not cross-compilation) would require building on Windows with Visual Studio, MSYS2, or similar Windows development environments.

## Testing

```bash
make test                 # Run unit/integration tests on Linux
make test-windows         # Run Windows x64 tests under Wine
make test-windows-x86     # Run Windows x86 tests under Wine
./scripts/test_windows.sh functional-only build/windows-release # Functional tests only
```

## Docker Support

```bash
docker build -t art2img .
docker run -v $(pwd):/app art2img make all
```

## Development Tools

### Code Formatting
```bash
make fmt         # Format sources in-place via clang-format
make fmt-check   # Verify formatting without modifying files
```

### Code Analysis
```bash
make lint        # Run clang-tidy analysis against all source files
```

### Verification
```bash
make verify      # Run comprehensive checks (build + test + lint + fmt-check)
```

## Installation

To install the binary to the system:

```bash
make install     # This will copy the binary to /usr/local/bin (may require sudo)
```

The install location can be customized using the DESTDIR variable:

```bash
make install DESTDIR=/custom/path
```
