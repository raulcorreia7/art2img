# Conventional Commits by Feature/Type/Group

## Core Library Components

### ART File Processing
* feat(art): implement ART file parsing and processing
* feat(art): add ART file header structure
* feat(art): add ART file tile structure
* feat(art): implement ART file loading functionality
* test(art): add tests for ART file parsing
* test(art): add tests for ART file validation

### Palette Handling
* feat(palette): implement palette management
* feat(palette): add palette loading functionality
* feat(palette): add palette validation
* test(palette): add tests for palette handling
* test(palette): add tests for palette validation

### Image Processing
* feat(image): implement image processing functionality
* feat(image): add RGBA conversion
* feat(image): add premultiplication support
* feat(image): add matte hygiene processing
* feat(image): add magenta detection for transparency
* test(image): add tests for image processing
* test(image): add tests for transparency handling

### Image Writing
* feat(writer): implement image writing functionality
* feat(writer): add PNG format support
* feat(writer): add TGA format support
* feat(writer): add BMP format support
* feat(writer): add format encoding to memory
* test(writer): add tests for PNG format
* test(writer): add tests for TGA format
* test(writer): add tests for BMP format

### API Interface
* feat(api): implement extractor API
* feat(api): add extraction result structure
* feat(api): add ArtView structure
* feat(api): add ImageView structure
* feat(api): add image format conversion
* test(api): add tests for extractor API
* test(api): add tests for image view functionality

### Exception Handling
* feat(exceptions): implement custom exception classes
* feat(exceptions): add ArtException base class
* test(exceptions): add tests for exception handling

### File Operations
* feat(files): implement file operations utilities
* feat(files): add TGA header structure
* feat(files): add BMP header structures
* test(files): add tests for file operations

### Core Utilities
* feat(colors): implement color output utilities
* feat(colors): add color guard functionality
* feat(threading): integrate BS thread pool
* feat(version): generate version information
* test(colors): add tests for color output

## CLI Components

### CLI Application
* feat(cli): implement CLI application builder
* feat(cli): add CLI operations interface
* feat(cli): add main entry point
* feat(cli): implement configuration processing
* feat(cli): add processing logic
* feat(cli): add options translation
* feat(cli): add processing modes
* test(cli): add tests for CLI options
* test(cli): add tests for CLI processing
* test(cli): add tests for CLI error handling
* test(cli): add tests for CLI processing order

## Testing Components

### Unit Tests
* test(core): add tests for core functionality
* test(core): add tests for image processing
* test(core): add tests for file operations
* test(core): add tests for API interface
* test(core): add tests for palette handling
* test(core): add tests for ART file processing
* test(core): add tests for exception handling
* test(core): add tests for color utilities
* test(core): add tests for image writing
* test(core): add tests for transparency processing

### Test Helpers
* test(helpers): add test helper utilities
* test(helpers): add test asset loading
* test(helpers): add test asset verification

### Integration Tests
* test(integration): add BATS integration tests
* test(integration): add conversion tests
* test(integration): add transparency tests
* test(integration): add Windows compatibility tests

## Build and Configuration Components

### Build System
* build(cmake): configure CMake build system
* build(cmake): add library target configuration
* build(cmake): add CLI target configuration
* build(cmake): add test target configuration
* build(cmake): add diagnostic tool configuration
* build(cmake): add version generation
* build(cmake): add pkg-config support
* build(cmake): add clang-format targets
* build(cmake): add clang-tidy targets
* build(cmake): add optimization flags
* build(cmake): add sanitizer support
* build(cmake): add cross-compilation support
* build(cmake): add Windows toolchain
* build(cmake): add Windows x86 toolchain
* build(cmake): add dependency management
* build(make): implement Makefile build system
* build(make): add build targets
* build(make): add test targets
* build(make): add cross-compilation targets
* build(make): add release targets
* build(make): add clean targets
* build(make): add format targets
* build(make): add lint targets
* build(make): add installation targets
* build(scripts): add build scripts
* build(scripts): add cross-compilation scripts
* build(scripts): add native build scripts
* build(scripts): add test scripts
* build(scripts): add doctor script
* build(scripts): add BATS test runner
* chore(build): update build configuration
* ci(build): add CI configuration
* ci(github): add GitHub Actions workflow

## Documentation and Support Components

### Documentation
* docs(readme): add project documentation
* docs(readme): add quick start guide
* docs(readme): add command line options
* docs(readme): add build instructions
* docs(readme): add license information
* docs(readme): add credits
* docs(building): add detailed build instructions
* docs(pipeline): add pipeline requirements
* docs(pipeline): add pipeline notes
* docs(art): add ART format documentation
* chore(docs): update documentation