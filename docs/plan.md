# Delivery Plan — Oct 2025 refresh

## Goals
- Make the build/test cycle reproducible with CMake-managed third-party dependencies (FetchContent).
- Ensure palette handling matches Duke Nukem 3D assets byte-for-byte while keeping zero-copy behaviour.
- Replace brittle shell integration tests with fast C++ coverage.

## Scope
- `src/palette.cpp`, `include/palette.hpp`, `src/art_file.cpp`, `include/extractor_api.hpp` — raw palette storage, scaling, zero-copy caching.
- `CMakeLists.txt`, `tests/CMakeLists.txt`, dependency FetchContent blocks — doctest integration, asset sync.
- `tests/test_functionality.cpp` plus supporting helpers — new doctest coverage for CLI parity, palette comparison, and extraction.

## Out of Scope
- Shipping additional third-party source trees (e.g. vendor/eduke32 removed).
- Re-introducing Makefile/CI flows (legacy content preserved below for reference only).
- Modifying CLI option surface beyond palette-help text.

## Workstreams
- **T01 – Fetch doctest**: Manage doctest via FetchContent and expose `doctest::doctest_with_main` locally.
- **T02 – Palette/ART refactor**: Store raw 6-bit palette data, regenerate Build-style BGR, cache ART files for zero-copy extraction.
- **T03 – Test suite refresh**: Port shell scripts to C++ doctests (`tests/test_functionality.cpp`), extend helpers, prune legacy scripts.

## Tooling & Commands
- Configure/build: `cmake -S . -B build -DBUILD_TESTS=ON`, `cmake --build build`.
- Tests: `ctest --test-dir build --output-on-failure`.
- Manual extraction for sanity: `build/bin/art2img -o tests/output/tmp -f png -p tests/assets/PALETTE.DAT tests/assets/TILES000.ART`.

## Risks & Mitigations
- **Dependency drift**: pinned FetchContent revisions could lag upstream → documented in plan/memory bank.
- **Palette mismatch**: raw pointer comparisons ensure palette bytes remain aligned; tests guard regressions.
- **CLI parity**: doctest covers key CLI behaviours; fallback CLI invocation documented above.

## Rollback Strategy
- Revert FetchContent dependency block and restore local headers if offline builds become required again.
- Revert palette/raw caching commits (see CC list below) if legacy behaviour required.

---
(historical plans omitted for brevity)
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
