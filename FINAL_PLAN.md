# FINAL ARCHITECTURE & MIGRATION PLAN - art2img Library

## EXECUTIVE SUMMARY

This document outlines the complete migration from the legacy ExtractorAPI to a modern, memory-first architecture with clean separation of concerns, zero-copy operations, and composable functions.

## CURRENT STATE ANALYSIS

### Codebase Strengths:
- ✅ Modern C++20 with RAII, move semantics, and type safety
- ✅ Comprehensive file utilities with proper error handling
- ✅ Good encoder interface design
- ✅ Memory-first approach with ArtData RAII wrapper

### Critical Architectural Issues:
- ❌ **Duplication**: ArtFile and ArtData classes with identical parsing logic
- ❌ **Inconsistent Error Handling**: Mix of Result<T>, Error, and boolean returns
- ❌ **Mixed Responsibilities**: API functions handle file I/O, processing, and encoding
- ❌ **Tight Coupling**: Encoders depend directly on palette and conversion options
- ❌ **Memory Inefficiency**: Multiple data copies in processing pipeline

## ARCHITECTURE VISION & PRINCIPLES

### Core Principles:
1. **Stateless Functional API** - Pure functions with clear boundaries
2. **Memory-First Design** - All operations work on buffers before file I/O
3. **Clean Separation** - File I/O, data parsing, processing, and encoding in distinct layers
4. **Zero-Copy Operations** - Minimize data copying through views and streaming
5. **Consistent Error Handling** - Standard Result<T> pattern throughout (migrating from exceptions)

## ARCHITECTURE DIAGRAMS

### Current (Legacy) Architecture:
```
┌─────────────────────────────────────────────────────────┐
│                    LEGACY ARCHITECTURE                   │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌─────────────────┐  ┌─────────────────┐               │
│  │   ExtractorAPI  │  │   ArtFile       │               │
│  │  (Monolithic)   │  │  (File-based)   │               │
│  └─────────────────┘  └─────────────────┘               │
│         │                      │                        │
│         ▼                      ▼                        │
│  ┌─────────────────┐  ┌─────────────────┐               │
│  │ ImageProcessing │  │ FileOperations  │               │
│  │ (Mixed concerns)│  │ (Mixed I/O)     │               │
│  └─────────────────┘  └─────────────────┘               │
│         │                      │                        │
│         ▼                      ▼                        │
│  ┌─────────────────┐  ┌─────────────────┐               │
│  │ Format Writers  │  │    CLI App      │               │
│  │ (write_png(),   │  │ (Tight coupling)│               │
│  │  write_tga())   │  │                 │               │
│  └─────────────────┘  └─────────────────┘               │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Target Architecture:
```
┌─────────────────────────────────────────────────────────┐
│                    TARGET ARCHITECTURE                   │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌─────────────────────────────────────────────────────┐│
│  │                  PUBLIC API LAYER                   ││
│  │  extract_tile()  write()  encode()  process_file()  ││
│  └─────────────────────────────────────────────────────┘│
│                         │                                │
│                         ▼                                │
│  ┌─────────────────────────────────────────────────────┐│
│  │                 PROCESSING LAYER                    ││
│  │  TileExtractor  ColorConverter  ImageProcessor      ││
│  └─────────────────────────────────────────────────────┘│
│                 │                   │                   │
│                 ▼                   ▼                   │
│  ┌─────────────────┐  ┌────────────────────────────────┐│
│  │  DATA PARSING   │  │        FORMAT ENCODING         ││
│  │  ArtParser      │  │  PNGEncoder  TGAEncoder  ...   ││
│  │  ArtContainer   │  └────────────────────────────────┘│
│  └─────────────────┘                   │                │
│                 │                      ▼                │
│                 ▼              ┌─────────────────┐      │
│          ┌─────────────┐       │   FILE I/O      │      │
│          │ MEMORY DATA │       │ file_utilities  │      │
│          └─────────────┘       │   file_writer   │      │
│                 │              └─────────────────┘      │
│                 ▼                      │                │
│          ┌─────────────┐              ▼                │
│          │  FILE DATA  │       ┌─────────────┐         │
│          └─────────────┘       │  FILESYSTEM │         │
│                                 └─────────────┘         │
└─────────────────────────────────────────────────────────┘
```

## ARCHITECTURE LAYERS

### Layer 1: File I/O Operations (COMPLETE)
- `file_utilities.hpp/cpp` - Comprehensive file loading/saving with caching
- `io/file_writer.hpp/cpp` - Simple atomic file writing
- **Status**: Well-implemented, no changes needed

### Layer 2: Data Parsing & Loading (NEEDS REFACTORING)
- **ArtParser** - Pure functions for ART header/tile metadata parsing
- **ArtContainer** - RAII wrapper consolidating ArtFile/ArtData functionality
- **PaletteLoader** - Palette loading and management utilities
- **Goal**: Eliminate duplication, standardize error handling

### Layer 3: Core Processing (NEEDS EXTRACTION)
- **TileExtractor** - Extract tile data from ART containers using zero-copy views
- **ColorConverter** - Stateless palette application with transparency handling
- **ImageProcessor** - Generic image manipulation utilities
- **Goal**: Separate processing from file I/O and encoding

### Layer 4: Format Encoding (GOOD FOUNDATION)
- `encoder.hpp` - Unified format-agnostic interface
- `png_encoder.hpp/cpp` - Memory-only PNG encoding
- `tga_encoder.hpp/cpp` - Memory-only TGA encoding  
- `bmp_encoder.hpp/cpp` - Memory-only BMP encoding
- **Goal**: Decouple from palette dependency

### Layer 5: Public API (NEEDS SIMPLIFICATION)
- `art2img.hpp` - Stateless, composable functions:
  - `parse_art_data()` - Pure ART parsing from memory
  - `extract_tile()` - Memory-based tile extraction
  - `convert_to_rgba()` - Color conversion with palette
  - `encode_image()` - Format encoding from RGBA data
  - `process_art_file()` - Complete file-to-file processing
- **Goal**: Memory-first, composable interface

## FOLDER STRUCTURE MIGRATION

### Phase 0: Current State (Legacy)
```
include/art2img/
├── extractor_api.hpp    ← LEGACY: Monolithic stateful API
├── image_processing.hpp ← LEGACY: Mixed concerns
├── file_operations.hpp  ← LEGACY: Mixed I/O
└── exceptions.hpp       ← LEGACY: Exception-based errors

src/
├── extractor_api.cpp    ← LEGACY
├── image_processing.cpp ← LEGACY  
├── file_operations.cpp  ← LEGACY
└── exceptions.cpp       ← LEGACY
```

### Phase 1: Foundation (New Structure)
```
include/art2img/
├── api/                 ← NEW: Public interface
│   ├── art2img.hpp      ← Stateless functions
│   └── discovery.hpp    ← File scanning utilities
├── core/                ← NEW: Core data types
│   ├── types.hpp        ← Standard types & errors
│   ├── art_data.hpp     ← RAII ART container
│   ├── image.hpp        ← Image data container
│   └── palette.hpp      ← Palette management
├── formats/             ← NEW: Format encoding
│   ├── encoder.hpp      ← Unified interface
│   ├── png_encoder.hpp
│   ├── tga_encoder.hpp
│   └── bmp_encoder.hpp
└── io/                  ← NEW: Pure I/O
    └── file_writer.hpp  ← Atomic file operations

src/
├── api/
│   ├── art2img.cpp      ← Public API implementation
│   └── discovery.cpp    ← Discovery implementation
├── core/
│   ├── art_data.cpp     ← ART container implementation
│   ├── image.cpp        ← Image implementation
│   └── palette.cpp      ← Palette implementation
├── formats/
│   ├── encoder_factory.cpp
│   ├── png_encoder.cpp
│   ├── tga_encoder.cpp
│   └── bmp_encoder.cpp
└── io/
    └── file_writer.cpp  ← File I/O implementation
```

### Phase 2: Processing Layer Extraction
```
include/art2img/core/
├── processing/          ← NEW: Pure processing functions
│   ├── art_parser.hpp   ← ART parsing utilities
│   ├── tile_extractor.hpp ← Tile extraction
│   ├── color_converter.hpp ← Palette application
│   └── image_processor.hpp ← Generic image ops

src/core/processing/
├── art_parser.cpp
├── tile_extractor.cpp
├── color_converter.cpp
└── image_processor.cpp
```

### Phase 3: Backward Compatibility Layer
```
include/art2img/legacy/    ← TEMPORARY: Compatibility
├── extractor_adapter.hpp   ← Maps old API to new
└── types.hpp              ← Legacy type aliases

src/legacy/
└── extractor_adapter.cpp  ← Implementation
```

### Phase 4: Final Cleanup
- Remove `legacy/` directory
- Remove old files: `extractor_api.*`, `image_processing.*`, etc.
- Update CLI to use new API directly

## PUBLIC API CONTRACT

### Core Functions:
```cpp
// Memory-first processing pipeline
Result<ArtData> load_art_data(const std::vector<uint8_t>& buffer);
Result<ImageData> extract_tile(const ArtData& art, uint32_t index);
Result<std::vector<uint8_t>> apply_palette(const ImageData& image, const Palette& palette, 
                                          ConversionOptions options = {});
Result<std::vector<uint8_t>> encode_image(const ImageData& image, ImageFormat format);

// Complete file processing (composes above functions)
Result<void> process_art_file(const path& art_path, const path& output_path, 
                             uint32_t tile_index, ImageFormat format,
                             ConversionOptions options = {});
```

### Data Structures:
```cpp
struct ArtData {
    std::vector<uint8_t> buffer;
    Header header;
    std::vector<TileInfo> tiles;
    bool is_valid;
};

struct ImageData {
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> pixels; // Indexed or RGBA data
    bool has_alpha;
};

struct ConversionOptions {
    bool enable_alpha = true;
    bool fix_transparency = true;
    bool premultiply_alpha = false;
};
```

## DATA FLOW

### Current (Legacy) Data Flow:
```
ART File → ExtractorAPI.load() → Process → write_png()/write_tga() → File
```

### New (Memory-First) Data Flow:
```
ART File → file_utilities.load_file() → ArtData → TileExtractor → 
ColorConverter → PNGEncoder → file_writer.write() → Output File
```

### Composable Processing:
```cpp
// Memory-based processing pipeline
auto art_data = load_art_data(file_buffer);
auto tile = extract_tile(art_data, index);
auto rgba = apply_palette(tile, palette, options);
auto png_data = encode_image(rgba, Format::PNG);
write_file(output_path, png_data);

// Or single function for convenience
process_art_file(input_path, output_path, index, Format::PNG, options);
```

## MIGRATION TIMELINE & TASKS

### Week 1: Foundation & Consolidation
- **Task 1.1**: Create unified `ArtContainer` replacing ArtFile/ArtData
- **Task 1.2**: Standardize error handling with `Result<T>` pattern
- **Task 1.3**: Extract pure `ArtParser` functions for memory-based parsing
- **Task 1.4**: Set up new directory structure with `core/`, `formats/`, `io/`, `api/`

### Week 2: Processing Layer Extraction
- **Task 2.1**: Implement `TileExtractor` with zero-copy tile data access
- **Task 2.2**: Create `ColorConverter` for stateless palette application
- **Task 2.3**: Develop `ImageProcessor` for generic image operations
- **Task 2.4**: Decouple encoders from palette dependency

### Week 3: Memory-First API & Encoding
- **Task 3.1**: Design composable memory-based API functions
- **Task 3.2**: Implement streaming processing to minimize data copying
- **Task 3.3**: Enhance format encoders for memory-only operation
- **Task 3.4**: Add comprehensive unit tests for all new components

### Week 4: Public API & Integration
- **Task 4.1**: Implement stateless public API in `art2img.hpp`
- **Task 4.2**: Create discovery utilities for file scanning
- **Task 4.3**: Develop backward compatibility adapter layer
- **Task 4.4**: Update build system for new structure

### Week 5: CLI Migration & Testing
- **Task 5.1**: Update CLI to use new stateless API
- **Task 5.2**: Implement comprehensive integration tests
- **Task 5.3**: Performance benchmarking and optimization
- **Task 5.4**: Documentation and usage examples

### Week 6: Cleanup & Optimization
- **Task 6.1**: Remove legacy ExtractorAPI and related files
- **Task 6.2**: Performance optimization with zero-copy operations
- **Task 6.3**: Final code review and cleanup
- **Task 6.4**: Release preparation and documentation finalization

## BACKWARD COMPATIBILITY

### Compatibility Layer:
```cpp
// legacy_api.hpp
class LegacyExtractor {
public:
    // Maps old API to new composable functions
    bool load_art_file(const path& filename);
    ExtractionResult extract_tile(uint32_t index, ImageFormat format);
};
```

### Legacy ExtractorAPI Implementation:

The legacy `extractor_api.hpp/cpp` should be maintained as a self-contained compatibility layer that provides the exact same interface as the original ExtractorAPI but internally redirects to the new composable functions.

```cpp
// extractor_api.hpp - Self-contained legacy compatibility layer
#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "art2img/core/types.hpp"

// Maintain exact original ExtractorAPI interface
class ExtractorAPI {
public:
    ExtractorAPI();
    ~ExtractorAPI();
    
    // Original API methods
    bool load_art_file(const std::filesystem::path& filename);
    bool is_loaded() const;
    uint32_t tile_count() const;
    ExtractionResult extract_tile(uint32_t tile_index, ImageFormat format);
    Error write_tile(const std::filesystem::path& output_path, 
                    uint32_t tile_index, ImageFormat format);
    
    // Additional original methods...
    
private:
    // Internal state that maps to new architecture
    std::unique_ptr<ArtData> art_data_;
    Palette palette_;
    bool loaded_;
};
```

### Implementation Strategy:

```cpp
// extractor_api.cpp - Maps old API to new composable functions
#include "extractor_api.hpp"
#include "art2img/api/art2img.hpp"
#include "art2img/core/art_data.hpp"
#include "art2img/core/palette.hpp"

ExtractorAPI::ExtractorAPI() : loaded_(false) {}

bool ExtractorAPI::load_art_file(const std::filesystem::path& filename) {
    // Use new memory-first API internally
    auto result = art2img::load_art_data(filename);
    if (result.is_error()) {
        loaded_ = false;
        return false;
    }
    
    art_data_ = std::move(result.value);
    palette_.load_duke3d_default(); // Load default palette
    loaded_ = true;
    return true;
}

ExtractionResult ExtractorAPI::extract_tile(uint32_t tile_index, ImageFormat format) {
    if (!loaded_ || !art_data_) {
        return ExtractionResult{Error::fail(ErrorCode::InvalidState, "No ART file loaded")};
    }
    
    // Use new composable functions
    ConversionOptions options;
    options.enable_alpha = true;
    options.fix_transparency = true;
    
    return art2img::extract_tile_from_memory(*art_data_, tile_index, format, options);
}

Error ExtractorAPI::write_tile(const std::filesystem::path& output_path,
                              uint32_t tile_index, ImageFormat format) {
    auto result = extract_tile(tile_index, format);
    if (result.error.hasError()) {
        return result.error;
    }
    
    // Use new file writer
    return art2img::io::write_file(output_path, result.image_data);
}
```

### Key Requirements for Legacy Layer:
1. **Exact Interface Match**: Maintain identical method signatures
2. **Self-Contained**: No dependencies on internal new architecture details
3. **Error Mapping**: Convert new error types to legacy error formats
4. **State Management**: Maintain internal state that mimics original behavior
5. **Performance**: Minimal overhead from redirection layer

This compatibility layer ensures that existing code continues to work unchanged during the migration period while internally using the new architecture.

### Migration Path:
1. New code uses memory-first composable API
2. Existing code works via compatibility layer during transition
3. Gradual internal migration to new architecture
4. Final removal of legacy API after 2-3 release cycles

## ERROR HANDLING MIGRATION STRATEGY

### Current State Analysis:
The codebase currently has **inconsistent error handling patterns**:
- `file_utilities.cpp` uses `Result<T>` with `FileError` enum
- `api/art2img.cpp` uses `Error` with `ErrorCode` enum  
- `art_file.cpp` uses boolean returns with `std::cerr` output
- Some components still use C++ exceptions

### Migration to Consistent Result<T> Pattern:

1. **Standardize on Single Error Type**:
   ```cpp
   // Unified error type for entire codebase
   struct Error {
       ErrorCode code;
       const char* message;
       
       static Error ok();
       static Error fail(ErrorCode c, const char* msg);
       bool hasError() const;
   };
   ```

2. **Result<T> Template**:
   ```cpp
   template <typename T>
   struct Result {
       T value;
       Error error;
       
       Result(T&& val);
       Result(Error err);
       bool is_error() const;
   };
   ```

3. **Exception to Result<T> Conversion**:
   - Remove all `throw` statements
   - Convert `try/catch` blocks to return `Result<T>` with appropriate errors
   - External library exceptions should be caught and converted to `Error`

4. **Error Code Standardization**:
   ```cpp
   enum class ErrorCode {
       None,
       FileNotFound,
       InvalidFormat, 
       CorruptedData,
       MemoryError,
       InvalidArgument,
       UnsupportedOperation
   };
   ```

### Migration Steps:
1. **Phase 1**: Convert file utilities to use unified `Error` type
2. **Phase 2**: Update API layer to use consistent `Result<T>`
3. **Phase 3**: Remove all exception usage from core processing
4. **Phase 4**: Ensure external library exceptions are properly handled

### Benefits:
- **No exceptions** - Eliminates exception overhead and complexity
- **Contextual errors** - Detailed error messages with file paths and context
- **Recoverable errors** - Continue processing on per-tile failures
- **Cross-platform consistency** - Same error handling on all platforms

## PERFORMANCE TARGETS

- **30% reduction** in memory usage through zero-copy operations
- **No performance regression** in processing speed
- **Linear scaling** with multi-threading support
- **Minimal allocations** through buffer reuse and streaming

## RISK MITIGATION

- **Incremental Implementation**: Phase-based approach with testing at each stage
- **Compatibility Layer**: Temporary wrapper during CLI transition
- **Comprehensive Testing**: Maintain 100% test coverage during migration
- **Performance Monitoring**: Benchmark critical paths after each change
- **Version Control**: Use git branches for safe experimentation

## ADDITIONAL INSIGHTS & CLEANUPS

### Code Quality Improvements:

1. **Eliminate Direct Console Output**:
   - Remove all `std::cerr` usage from core processing code
   - Replace with proper error return values in `Result<T>`
   - Console output should only be in CLI layer, not library core

2. **Header Include Cleanup**:
   - Audit and minimize external includes in public headers
   - Use forward declarations where possible
   - Ensure all includes are necessary and properly scoped

3. **Memory Management**:
   - Replace manual `new` with `std::make_unique` where appropriate
   - Ensure all resource management follows RAII principles
   - Add memory usage tracking for large operations

4. **Build System Optimization**:
   - Review CMake configuration for unnecessary dependencies
   - Optimize build times with proper target dependencies
   - Ensure cross-platform compatibility testing

### Testing & Documentation:

1. **Test Coverage Expansion**:
   - Add tests for error conditions and edge cases
   - Implement property-based testing for core algorithms
   - Add performance regression tests
   - Ensure 100% coverage of public API

2. **Documentation Completeness**:
   - Add API documentation for all public functions
   - Create usage examples for common scenarios
   - Document architecture decisions and trade-offs
   - Add contributor guidelines

3. **Benchmarking Suite**:
   - Create comprehensive performance benchmarks
   - Track memory usage, processing speed, and scalability
   - Establish performance baselines for regression testing

### Security & Robustness:

1. **Input Validation**:
   - Add comprehensive input validation for all public APIs
   - Validate file paths, buffer sizes, and parameter ranges
   - Prevent buffer overflows and other security vulnerabilities

2. **Error Recovery**:
   - Implement graceful error recovery where possible
   - Add retry mechanisms for transient failures
   - Ensure proper resource cleanup on errors

3. **Cross-Platform Compatibility**:
   - Test on all supported platforms (Windows, Linux, macOS)
   - Handle platform-specific file system differences
   - Ensure consistent behavior across environments

## SUCCESS CRITERIA

### API Quality:
- Single `write()` function with format parameter
- Clean separation between memory and file operations
- No `write_x/write_y` duplicate functions
- Professional C++ code style throughout
- Comprehensive input validation and error handling

### Performance:
- No regression in processing speed
- **30% reduction** in memory usage through zero-copy operations
- Linear scaling with multi-threading support
- Minimal allocations through buffer reuse and streaming

### Maintainability:
- Clear architecture documentation
- **95%+ test coverage** of all functionality
- Minimal complexity as requested
- Comprehensive error handling and recovery
- Cross-platform compatibility

### Security:
- No known security vulnerabilities
- Comprehensive input validation
- Proper resource cleanup on all code paths
- Memory safety guarantees

This comprehensive plan transforms the art2img library from a monolithic, stateful architecture to a clean, composable, memory-first design with proper separation of concerns, professional API design, and enterprise-grade reliability.
