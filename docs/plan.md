# Current Delivery Plan — 2025-10-02

## Goals
- Restore end-to-end CI so release artifacts build for Linux/Windows without manual intervention.
- Eliminate in-memory PNG corruption so the embeddable API remains byte-for-byte compatible with CLI output.
- Tighten platform guards (thread defaults, header includes) without changing the public surface area.

## Scope
- `.github/workflows/build.yml` verification logic and supporting build scripts.
- `src/png_writer.cpp` / `include/png_writer.hpp` and API-level tests covering in-memory extraction.
- `include/extractor.hpp`, `src/art2img.cpp`, and supporting utilities for safer thread-count defaults / palette resolution reporting.

## Out of Scope
- Changing CLI flags, file formats, or default palette heuristics.
- Replacing the Makefile or switching build toolchains.
- Introducing new third-party dependencies beyond vendored headers.

## Workstreams
- **T01 – Pipeline verification fix**: Correct binary naming in workflow, align Makefile verify target with CI usage, add guardrails so missing MinGW toolchain degrades gracefully. _Owner: Codex._
- **T02 – In-memory PNG buffer integrity**: Fix chunk handling in `PngWriter::write_png_to_memory`, add regression test in `tests/test_library_api.cpp` (or new binary) ensuring multi-chunk writes round-trip. _Owner: Codex._
- **T03 – Extractor robustness**: Ensure thread defaults never drop to zero, add explicit `<thread>` include, surface helpful logging when palette fallback fails. _Owner: Codex._

## Tooling & Commands
- Build: `make`, `make linux`, `make windows` (optional when `x86_64-w64-mingw32-g++` present).
- Verification: `make verify` (delegates to `verify-linux` + `verify-windows`), `make test`, `./tests/test_ci.sh`.
- Docker (optional parity): `docker build -t art2img .`.

## Risks & Mitigations
- **Workflow drift**: Misnaming binaries leads to hard failures → add CI assertion + local `make verify` parity command.
- **PNG regression**: stb callback behaviour may still vary → exercise regression test with synthetic large tile to ensure multi-chunk output.
- **Thread defaults**: Hardware concurrency returning 0 would halt extraction → clamp minimum to 1 with unit smoke verifying CLI path.

## Rollback Strategy
- Each change isolated by task; revert offending commit or cherry-pick revert.
- Release assets built from prior successful tag remain usable; pipeline fix guarded behind matrix so fallback is manual `make linux` + upload.

---

## Historical Plan (previous content)
# Library Refactoring Plan: Memory-Based C++ API

Based on the current art2img implementation, this plan outlines the transformation of the file-based tool into a stable C++ library API with memory-based operations.

## Current Architecture Analysis
- **File-based operations**: All current operations use `std::ifstream` and file I/O
- **Tight coupling**: ArtFile, Palette, and Writers are tightly coupled to file system
- **No memory API**: Missing memory buffer support needed for Duke3D GRP integration

## Phase 1: Core Memory-Based Infrastructure

### ArtFile Memory API
- Add `load_from_memory(const uint8_t* data, size_t size)` method
- Replace `std::ifstream` with memory buffer operations
- Maintain backward compatibility with file-based API

### Palette Memory API
- Add `load_from_memory(const uint8_t* data, size_t size)` method
- Support both Duke3D and Blood palette formats
- Preserve existing file-based loading

## Phase 2: Memory-Based Image Writers

### PNG Writer Memory API
- Add `write_png_to_memory()` using STB Image Write callbacks
- Support RGBA output with alpha transparency
- Return `std::vector<uint8_t>` with PNG data

### TGA Writer Memory API
- Add `write_tga_to_memory()` method
- Support both RGB and RGBA formats
- Return `std::vector<uint8_t>` with TGA data

## Phase 3: High-Level API Layer

### ExtractorAPI Class
- Design clean, memory-based extraction interface
- Support single tile and batch extraction
- Return structured `ExtractionResult` objects

### ExtractionResult Structure
- Contain tile data, animation info, and image buffers
- Support error handling and status reporting
- Enable Duke3D GRP pipeline integration

## Phase 4: Build System & Documentation

### Library Targets
- Static library (`libart2img.a`)
- Shared library (`libart2img.so`)
- Update Makefile with proper flags

### API Documentation
- Comprehensive usage examples
- Duke3D GRP integration guide
- Memory management guidelines

## Key Technical Decisions

1. **Memory Ownership**: Use RAII patterns, avoid raw pointers
2. **API Design**: Clean separation between file and memory APIs
3. **Backward Compatibility**: Maintain existing CLI functionality
4. **Error Handling**: Consistent exception/return value patterns
5. **Thread Safety**: Preserve existing thread pool architecture

The library refactoring will enable seamless integration with the Duke3D upscaling pipeline while maintaining all existing functionality.

## Original Implementation Plan

Here is a **C++-native implementation plan** that integrates cleanly into an **existing C++ application** (e.g., one that already converts ART → PNG), and produces **fully transparent, EDuke32/BuildGDX–compatible PNG32 files**.

The plan is split into two phases as requested, with **Phase 2 designed to work with any upscaler—even those that ignore alpha**.

We assume your app already:
- Reads `PALETTE.DAT`,
- Decodes ART tiles (column-major),
- Outputs RGB PNGs with **magenta (`#FC00FC`/`#FF00FF`) as transparency**,
- Uses a PNG library (e.g., **libpng**, **lodepng**, or **stb_image_write**).

---

## 🎯 Final Output Requirements (Non-Negotiable)
- **PNG32 (RGBA, 8-bit)**,
- **Straight (non-premultiplied) alpha** (per PNG spec [Nigel Tao]),
- **α = 0** where input was magenta,
- **RGB = (0,0,0)** wherever α = 0,
- **Opaque pixels = `(palette6[idx] << 2)`**,
- **No magenta stored anywhere**.

---

# ✅ PHASE 1: Produce Fully Transparent PNGs
*(Integrate into your existing ART→PNG exporter)*

### Modify your tile export function:

```cpp
// Input: decoded 8-bit RGB (from palette6 << 2), width, height
// Output: write PNG with alpha

void exportTileWithAlpha(
    const std::vector<uint8_t>& rgb, // size = W*H*3, RGBRGB...
    int width, int height,
    const std::string& outputPath)
{
    std::vector<uint8_t> rgba;
    rgba.reserve(width * height * 4);

    for (size_t i = 0; i < rgb.size(); i += 3) {
        uint8_t r = rgb[i + 0];
        uint8_t g = rgb[i + 1];
        uint8_t b = rgb[i + 2];

        // Detect magenta: tolerant of 252/255
        bool isMagenta = (r >= 250) && (b >= 250) && (g <= 5);

        uint8_t a = isMagenta ? 0 : 255;

        // Sanitize: zero RGB under transparency
        if (a == 0) {
            r = g = b = 0;
        }

        rgba.push_back(r);
        rgba.push_back(g);
        rgba.push_back(b);
        rgba.push_back(a);
    }

    // Write RGBA PNG using your existing PNG backend
    writePNG32(outputPath, rgba.data(), width, height);
}
```

> ✅ **Result**: `tileXXXX.png` is now **PNG32, straight-alpha, EDuke32-ready** if no upscaling is used.

---

# 🔄 PHASE 2: Generic Upscaling Pipeline
*(Works with ANY upscaler—even RGB-only)*

This phase is **optional** and only runs if you enable upscaling.

### Step 2.1: Preprocess for RGB-Only Upscaler
**Goal**: Create a temporary RGB PNG safe for upscaling.

```cpp
void prepareForRGBUpscaler(
    const std::string& cleanPngPath,
    const std::string& rgbOutPath)
{
    // Load clean PNG (RGBA)
    int w, h;
    std::vector<uint8_t> rgba = loadPNG32(cleanPngPath, w, h); // implement with libpng/lodepng

    std::vector<uint8_t> rgb;
    rgb.reserve(w * h * 3);

    for (size_t i = 0; i < rgba.size(); i += 4) {
        uint8_t r = rgba[i + 0];
        uint8_t g = rgba[i + 1];
        uint8_t b = rgba[i + 2];
        uint8_t a = rgba[i + 3];

        if (a == 0) {
            // Fill with neutral gray to avoid chromatic bias
            r = g = b = 128;
        }

        rgb.push_back(r);
        rgb.push_back(g);
        rgb.push_back(b);
    }

    writePNG24(rgbOutPath, rgb.data(), w, h); // RGB-only PNG
}
```

### Step 2.2: Upscale (External Tool or Library)
- **Option A (external)**: Shell out to `realesrgan-ncnn-vulkan`, Waifu2x, etc.
  ```cpp
  system(("realesrgan-ncnn-vulkan -i " + rgbIn + " -o " + rgbOut).c_str());
  ```
- **Option B (embedded)**: Integrate an upscaler library (e.g., ncnn, OpenCV DNN).

> ⚠️ Upscaler **only sees RGB**. Alpha is handled separately.

### Step 2.3: Post-process — Restore Clean Alpha
**Goal**: Reapply transparency using **upscaled alpha mask**.

```cpp
void restoreAlphaAfterUpscale(
    const std::string& originalCleanPng,
    const std::string& upscaledRgbPng,
    const std::string& finalPng)
{
    // Load original clean PNG (for alpha)
    int origW, origH;
    auto origRgba = loadPNG32(originalCleanPng, origW, origH);
    std::vector<uint8_t> origAlpha;
    for (size_t i = 3; i < origRgba.size(); i += 4) {
        origAlpha.push_back(origRgba[i]);
    }

    // Load upscaled RGB
    int newW, newH;
    auto upscaledRgb = loadPNG24(upscaledRgbPng, newW, newH); // returns RGB vector

    // Upscale alpha to match new resolution (use simple box/nearest)
    std::vector<uint8_t> newAlpha = upscaleAlpha(origAlpha, origW, origH, newW, newH);

    // Zero out RGB where alpha = 0
    std::vector<uint8_t> finalRgba;
    finalRgba.reserve(newW * newH * 4);

    for (int i = 0; i < newW * newH; ++i) {
        uint8_t r = upscaledRgb[i * 3 + 0];
        uint8_t g = upscaledRgb[i * 3 + 1];
        uint8_t b = upscaledRgb[i * 3 + 2];
        uint8_t a = newAlpha[i];

        if (a == 0) {
            r = g = b = 0;
        }

        finalRgba.push_back(r);
        finalRgba.push_back(g);
        finalRgba.push_back(b);
        finalRgba.push_back(a);
    }

    writePNG32(finalPng, finalRgba.data(), newW, newH);
}
```

#### Helper: `upscaleAlpha` (simple box filter)
```cpp
std::vector<uint8_t> upscaleAlpha(
    const std::vector<uint8_t>& alpha,
    int srcW, int srcH,
    int dstW, int dstH)
{
    std::vector<uint8_t> result(dstW * dstH);
    float scaleX = float(srcW) / dstW;
    float scaleY = float(srcH) / dstH;

    for (int y = 0; y < dstH; ++y) {
        for (int x = 0; x < dstW; ++x) {
            // Nearest neighbor (fast and sufficient for binary alpha)
            int srcX = std::min(int(x * scaleX), srcW - 1);
            int srcY = std::min(int(y * scaleY), srcH - 1);
            result[y * dstW + x] = alpha[srcY * srcW + srcX];
        }
    }
    return result;
}
```

> ✅ **Result**: `finalPng` is **PNG32, straight-alpha, no magenta, EDuke32-ready**.

---

## 📦 Integration into Existing App

### Add to your pipeline:
```cpp
// After ART decode → RGB
exportTileWithAlpha(rgb, w, h, "clean/tileXXXX.png");

if (enableUpscaling) {
    prepareForRGBUpscaler("clean/tileXXXX.png", "tmp/rgb_in.png");
    runUpscaler("tmp/rgb_in.png", "tmp/rgb_out.png"); // external or internal
    restoreAlphaAfterUpscale("clean/tileXXXX.png", "tmp/rgb_out.png", "final/tileXXXX.png");
}
```

### DEF Generation (C++ or external script)
- Output files named: `tile1170.png` or `tile1170_05.png`.
- Use filename parsing to generate DEF entries (as in Upscale Wiki batch script).

---

## 🔒 Quality Enforcement (Add as asserts or logs)
- After `exportTileWithAlpha`:
  `assert( (a == 0) → (r == 0 && g == 0 && b == 0) )`
- After `restoreAlphaAfterUpscale`: same check.
- Validate no pixel has `(r≥250 && b≥250 && g≤5)`.

---

## 📚 Dependencies
- **PNG I/O**: `lodepng` (single-header, no external deps) or `libpng`.
- **Upscaler**: optional; can be external process.
- **No OpenCV required** (unless you embed upscaling).

---

## ✅ Why This Works
- **Phase 1** satisfies PNG spec and EDuke32.
- **Phase 2** is **agnostic to upscaler internals** — even works with tools that discard alpha.
- **No premultiplication ever used** → avoids dark halos and PNG spec violations.
- **Magenta never reaches upscaler** → no bleed.

This plan is **minimal, safe, and production-ready** for integration into any C++ ART-processing toolchain.
