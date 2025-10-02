# 01 Version Synchronization

## Intent
Make Makefile the single source of truth for version information.

## Changes
1. **Add version header generation to Makefile**
2. **Update src/art2img.cpp to use generated header**
3. **Add verify-version target**

## Files
- `Makefile` - Add header generation rule
- `include/version.hpp` - Generated during build
- `src/art2img.cpp` - Use version header

## Acceptance
- Version only defined in Makefile
- Generated header works
- make verify-version passes