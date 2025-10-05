# Building art2img

## 🛠️ Requirements

- **C++20 compiler** (GCC 10+, Clang 10+, MSVC 2019+)
- **CMake** 3.14+
- **pthread** (Linux only)

## 🏗️ Build Instructions

### Quick Build
```bash
# Clone and build
git clone https://github.com/raulcorreia7/art2img.git
cd art2img
make all
```

### Makefile Commands
```bash
make all          # Build for Linux (default)
make build        # Build for Linux
make windows      # Cross-compile for Windows x64
make windows-x86  # Cross-compile for Windows x86
make linux-release       # Release build + tests for Linux
make windows-release     # Release build for Windows x64
make windows-x86-release # Release build for Windows x86
make test         # Run tests on Linux
make test-windows # Test Windows x64 build (requires Wine)
make test-windows-x86 # Test Windows x86 build (requires Wine)
make clean        # Clean build directory
make install      # Install to system
make doctor       # Check host dependencies
```

### CMake Direct
```bash
mkdir -p build/linux-release && cd build/linux-release
cmake ../.. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

## 🖥️ Cross-Platform

### Windows (Cross-compilation)
```bash
make windows-release
# or configure manually
mkdir -p build/windows && cd build/windows
cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../cmake/windows-toolchain.cmake -DBUILD_SHARED_LIBS=OFF
cmake --build . --parallel
```

### Windows (Native)
```bash
# Use Visual Studio or MinGW
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --parallel
```

## 🧪 Testing

```bash
make test                 # Run unit/integration tests on Linux
make test-windows         # Run Windows x64 tests under Wine
make test-windows-x86     # Run Windows x86 tests under Wine
./scripts/test_windows.sh functional-only build/windows-release # Functional tests only
```

## 🐳 Docker

```bash
docker build -t art2img .
docker run -v $(pwd):/app art2img make all
```
