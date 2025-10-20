# Naming Conventions

This document establishes consistent naming conventions for the art2img library to improve code readability and maintainability.

## General Principles

1. **Clarity over brevity** - Names should clearly express intent
2. **Consistency** - Follow established patterns throughout the codebase
3. **Discoverability** - Names should be predictable and easy to find

## Function Naming

### Getters and Accessors
- Use `get_` prefix for functions that retrieve data
- Example: `get_tile()`, `get_palette_entry()`

### Factory Functions
- Use `make_` or `create_` prefix for functions that construct objects
- Example: `make_tile_view()`, `create_image()`

### Loading Functions
- Use `load_` prefix for functions that load data from external sources
- Example: `load_art_bundle()`, `load_palette()`

### Conversion Functions
- Use `to_` or `convert_to_` prefix for functions that transform data
- Example: `to_rgba()`, `convert_to_color()`

### Validation Functions
- Use `is_` prefix for boolean predicate functions
- Example: `is_valid()`, `is_transparent()`

### File Operations
- Use descriptive verbs for file-related operations
- Example: `discover_sidecar_palette()`, `export_tiles()`

## Variable Naming

### Local Variables
- Use descriptive names that clearly indicate purpose
- Prefer full words over abbreviations when clarity is improved
- Example: `tile_width` instead of `tw`

### Constants
- Use `UPPER_SNAKE_CASE` for constants
- Example: `MAX_TILE_WIDTH`, `PALETTE_SIZE`

### Class Members
- Use `snake_case` with descriptive names
- Avoid unnecessary prefixes
- Example: `tile_count`, `pixel_data`

## Type Naming

### Classes and Structs
- Use `PascalCase` for class and struct names
- Example: `ArtData`, `TileView`

### Enums
- Use `PascalCase` for enum names
- Use `snake_case` for enum values
- Example: `ImageFormat::png`, `PaletteHint::sidecar`

### Type Aliases
- Use `snake_case` for type aliases
- Example: `byte_span`, `u8_span`

## File Naming

### Header Files
- Use `snake_case.hpp` for header files
- Match header names to their primary class/namespace
- Example: `art.hpp`, `palette.hpp`

### Source Files
- Use `snake_case.cpp` for source files
- Match source file names to their corresponding headers
- Example: `art.cpp`, `palette.cpp`

## Namespace Naming

- Use `snake_case` for namespace names
- Keep namespaces short and descriptive
- Example: `art2img`, `art2img::detail`

## Consistency Guidelines

### Current Inconsistencies to Address

1. **Mixed function prefixes** - Some functions use `get_`, others use `make_`, and others use no prefix
   - Preferred pattern: Use appropriate prefix based on function purpose

2. **Inconsistent parameter naming** - Some parameters use abbreviations, others use full names
   - Preferred pattern: Use descriptive names that clearly indicate purpose

3. **Mixed constant naming** - Some constants use `UPPER_SNAKE_CASE`, others use `lowercase`
   - Preferred pattern: Use `UPPER_SNAKE_CASE` for all constants

## Future Improvements

As we refactor the codebase, we should gradually align all names with these conventions while maintaining backward compatibility through aliases where necessary.