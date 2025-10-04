# Module Architecture Documentation

## Overview

This document describes the new module architecture for the art2img library, which separates image processing from file operations. The refactoring was implemented to adhere to the Single Responsibility Principle (SRP) and improve code maintainability, testability, and flexibility.

## Architecture Overview

The new architecture consists of three distinct components:

1. **image_processor** - Pure image processing logic
2. **file_operations** - File I/O operations
3. **ImageWriter** - Coordinator class that maintains backward compatibility

### Data Flow

The data flow follows a clean separation pattern:
```
ART File Data → Memory Buffer → Image Processing → Processed Buffer → File Output
```

This architecture ensures that:
- Image processing operations are completely independent of file operations
- Each module can be tested in isolation
- New formats can be added without affecting existing code
- The system is more maintainable and extensible

## Module Details

### 1. Image Processor Module

The `image_processor` namespace contains all pure image processing algorithms, including color conversion, alpha handling, and premultiplication.

**Header File:** `include/image_processor.hpp`

**Key Responsibilities:**
- Convert indexed color data to RGBA format
- Apply alpha premultiplication
- Apply matte hygiene (erode + blur)
- Check for build engine magenta (transparency key)

**Public Interface:**
- `convert_to_rgba()` - Convert indexed color data to RGBA format
- `apply_premultiplication()` - Apply alpha premultiplication
- `apply_matte_hygiene()` - Apply alpha matte hygiene
- `is_build_engine_magenta()` - Check if color is magenta

**Usage:**
```cpp
#include "image_processor.hpp"

// Convert indexed color data to RGBA
auto rgba_data = art2img::image_processor::convert_to_rgba(
    palette, tile, pixel_data, pixel_data_size, options);

// Apply premultiplication
art2img::image_processor::apply_premultiplication(rgba_data);

// Apply matte hygiene
art2img::image_processor::apply_matte_hygiene(rgba_data, width, height);
```

### 2. File Operations Module

The `file_operations` namespace contains all file I/O operations for reading and writing different image formats.

**Header File:** `include/file_operations.hpp`

**Key Responsibilities:**
- PNG file operations (write to file/memory)
- TGA file operations (write to file/memory)
- BMP file operations (write to file/memory)
- Helper functions for encoding and format conversion

**Public Interface:**
- PNG operations:
  - `write_png_file()` - Write PNG to file
  - `encode_png_to_memory()` - Encode PNG to memory buffer
- TGA operations:
  - `write_tga_file()` - Write TGA to file
  - `encode_tga_to_memory()` - Encode TGA to memory buffer
- BMP operations:
  - `write_bmp_file()` - Write BMP to file
  - `encode_bmp_to_memory()` - Encode BMP to memory buffer

**Usage:**
```cpp
#include "file_operations.hpp"

// Write PNG to file
bool success = art2img::file_operations::write_png_file(
    filename, rgba_data, width, height);

// Encode PNG to memory
auto png_data = art2img::file_operations::encode_png_to_memory(
    rgba_data, width, height);
```

### 3. ImageWriter Coordinator Class

The `ImageWriter` class maintains the original public API and provides a compatibility layer for existing code.

**Header File:** `include/image_writer.hpp`

**Key Responsibilities:**
- Maintain backward compatibility with existing code
- Coordinate between image processing and file operations
- Provide a unified interface for image writing operations

**Usage:**
```cpp
#include "image_writer.hpp"

// Write to file (backward compatible)
art2img::ImageWriter::write_image(
    filename, format, palette, tile, pixel_data, size, options);

// Write to memory (backward compatible)
art2img::ImageWriter::write_image_to_memory(
    output, format, palette, tile, pixel_data, size, options);
```

## Code Examples

### Direct Module Usage

**Simple PNG Creation:**
```cpp
#include "image_processor.hpp"
#include "file_operations.hpp"

// Convert to RGBA
auto rgba_data = art2img::image_processor::convert_to_rgba(
    palette, tile, pixel_data, pixel_data_size, options);

// Write to file
art2img::file_operations::write_png_file(filename, rgba_data, width, height);

// Or encode to memory
auto png_data = art2img::file_operations::encode_png_to_memory(
    rgba_data, width, height);
```

**Advanced Processing with Options:**
```cpp
#include "image_processor.hpp"
#include "file_operations.hpp"

// Create options
art2img::ImageWriter::Options options;
options.enable_alpha = true;
options.premultiply_alpha = true;
options.matte_hygiene = true;
options.fix_transparency = true;

// Convert to RGBA with advanced processing
auto rgba_data = art2img::image_processor::convert_to_rgba(
    palette, tile, pixel_data, pixel_data_size, options);

// Write to file
art2img::file_operations::write_png_file(filename, rgba_data, width, height);
```

### Backward Compatibility

**Using ImageWriter (existing code):**
```cpp
#include "image_writer.hpp"

art2img::ImageWriter::Options options;
options.enable_alpha = true;
options.premultiply_alpha = true;

// Using existing API
bool success = art2img::ImageWriter::write_image(
    "output.png", art2img::ImageFormat::PNG, palette, tile, 
    pixel_data, pixel_data_size, options);

// Write to memory
std::vector<uint8_t> output;
bool success = art2img::ImageWriter::write_image_to_memory(
    output, art2img::ImageFormat::PNG, palette, tile,
    pixel_data, pixel_data_size, options);
```

## Module Interaction

The following diagram shows how the modules interact:

```
┌──────────────────┐
│   ImageWriter    │ (Coordinator)
│  (Compatibility) │
└─────────┬────────┘
          │
          ▼
┌──────────────────┐
│  ImageProcessor  │ (Pure image processing)
│   (Algorithms)   │
└─────────┬────────┘
          │
          ▼
┌──────────────────┐
│  FileOperations  │ (File I/O)
│   (I/O Handling) │
└──────────────────┘
```

## Migration Guide

### For Existing Code

If you are using the old `ImageWriter` class, you can continue to use it as before. The changes are backward compatible.

**Before:**
```cpp
#include "image_writer.hpp"
ImageWriter::write_image(filename, format, palette, tile, pixel_data, size, options);
```

**After:**
```cpp
#include "image_writer.hpp"
// Same code works as before
ImageWriter::write_image(filename, format, palette, tile, pixel_data, size, options);
```

### For New Code

For new code, consider using the individual modules directly for more control and flexibility.

**Using Direct Modules:**
```cpp
#include "image_processor.hpp"
#include "file_operations.hpp"

// Process image
auto rgba_data = art2img::image_processor::convert_to_rgba(
    palette, tile, pixel_data, size, options);

// Write file
art2img::file_operations::write_png_file(filename, rgba_data, width, height);
```

### Benefits of the New Architecture

1. **Separation of Concerns**: Each module has a single responsibility
2. **Improved Testability**: Image processing can be tested without file system dependencies
3. **Enhanced Performance**: Direct memory operations without unnecessary wrapper overhead
4. **Flexibility**: Easy to use individual components for specific tasks
5. **Maintainability**: Smaller, focused code files are easier to maintain
6. **Extensibility**: Adding new formats or processing features is straightforward

## Breaking Changes

The refactored architecture does not introduce any breaking changes to the public API. The `ImageWriter` class maintains the same interface as before, ensuring backward compatibility.

However, the internal implementation has been changed to delegate functionality to the new modules, which improves performance characteristics and error handling.

## Performance Considerations

The new architecture is designed to be more efficient than the previous implementation by:
- Eliminating code duplication between file and memory operations
- Reducing the number of intermediate data copies
- Providing more direct control over memory operations
- Enabling better optimization opportunities in the individual modules

## Testing and Validation

The new architecture has been thoroughly tested with:
- Unit tests for each individual module
- Integration tests for the complete pipeline
- Performance benchmarks comparing the old and new implementations
- Regression tests to ensure compatibility with existing code

## Conclusion

The new module architecture provides a clean, maintainable separation of concerns while maintaining backward compatibility. Developers can use the high-level `ImageWriter` class for simple operations or directly use the individual modules for more control and performance.

## Related Files

- [include/image_processor.hpp](include/image_processor.hpp)
- [include/file_operations.hpp](include/file_operations.hpp)
- [include/image_writer.hpp](include/image_writer.hpp)
- [src/image_processor.cpp](src/image_processor.cpp)
- [src/file_operations.cpp](src/file_operations.cpp)
- [src/image_writer.cpp](src/image_writer.cpp)
- [docs/image_writer_refactoring_plan.md](docs/image_writer_refactoring_plan.md)