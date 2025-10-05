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
make all                 # Build Linux Release (default)
make linux-release       # Build Linux Release
make linux-debug         # Build Linux Debug
make windows-release     # Build Windows Release
make windows-debug       # Build Windows Debug
make test-linux          # Run Linux tests
make test-windows        # Run Windows tests (requires Wine)
make clean               # Clean build directory
make install             # Install to system
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
# or
make build CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake"
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
make test-linux        # Run Linux tests
make test-windows      # Run Windows tests (requires Wine)
make test-asan         # Run with AddressSanitizer
make test-leak         # Run with LeakSanitizer
```

## 🐳 Docker

```bash
docker build -t art2img .
docker run -v $(pwd):/app art2img make all
```