# Pipeline Configuration Requirements

This project uses GitHub Actions for CI/CD. The following requirements must be met for the pipeline to work properly:

## Dependencies Required for GitHub Actions

The pipeline requires the following system dependencies to be installed in the runner:

- `build-essential`: For compilation tools (gcc, g++, make)
- `cmake`: For the build system
- `pkg-config`: For dependency management
- `g++-mingw-w64-i686`: MinGW cross-compiler for Windows x86
- `g++-mingw-w64-x86-64`: MinGW cross-compiler for Windows x64
- `wine`: To run Windows binaries during testing
- `zip`: For packaging operations

## Build Targets Supported in Pipeline

The pipeline uses the following make targets:

- `make build`: Builds the Linux version
- `make test`: Runs Linux unit tests
- `make windows`: Cross-compiles for Windows x64
- `make windows-x86`: Cross-compiles for Windows x86
- `make test-windows`: Runs Windows cross-compiled tests using Wine

## Wine Configuration

- The test script is configured to use the default Wine prefix
- Wine tests run from the build directory where assets are located
- WINEDEBUG environment variable is set to "-all" to suppress debug messages

## Test Assets

- The build process automatically copies test assets to the appropriate build directories
- Assets are located in the `tests/assets/` directory of the repository
- Both Linux and Windows builds have access to the same test assets

## Cross-Platform Compatibility

- The CMake build system handles cross-compilation for Windows using MinGW
- Test asset paths are resolved correctly for both native and cross-compiled environments
- The Windows test script properly handles path differences when running under Wine