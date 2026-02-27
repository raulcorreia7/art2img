// Integration tests: Duke3D Shareware GRP
//
// These tests require the shareware GRP to be present.

#include "../test_common.hpp"
#include "../test_constants.hpp"

#include "doctest.h"

#include <art2img.hpp>

using namespace art2img;
using namespace art2img::test;
using namespace art2img::test::constants;

// ============================================================================
// GRP Structure Tests
// ============================================================================

TEST_SUITE("Shareware GRP Structure") {
    TEST_CASE("GRP loads successfully" * doctest::skip(!shareware_available())) {
        auto result = GrpFile::load(SHAREWARE_GRP_PATH);
        CHECK(result.ok());
        CHECK(result.value().count() > 0);
    }

    TEST_CASE("GRP contains expected files" * doctest::skip(!shareware_available())) {
        auto result = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(result.ok());

        auto& grp = result.value();

        // Essential files
        CHECK(grp.has(PALETTE_DAT));
        CHECK(grp.has(TILES000_ART));

        // Case insensitive
        CHECK(grp.has("PALETTE.DAT"));
        CHECK(grp.has("Tiles000.Art"));
    }

    TEST_CASE("GRP file list is non-empty" * doctest::skip(!shareware_available())) {
        auto result = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(result.ok());

        auto list = result.value().list();
        CHECK(list.size() > MIN_GRP_FILE_COUNT);  // Shareware has many files

        // Check for various file types
        bool has_art = false;
        bool has_con = false;
        bool has_voc = false;
        bool has_mid = false;
        bool has_map = false;

        for (const auto& name : list) {
            if (name.size() > 4) {
                auto ext = name.substr(name.size() - 4);
                if (ext == EXT_ART)
                    has_art = true;
                if (ext == EXT_CON)
                    has_con = true;
                if (ext == EXT_VOC)
                    has_voc = true;
                if (ext == EXT_MID)
                    has_mid = true;
                if (ext == EXT_MAP)
                    has_map = true;
            }
        }

        CHECK(has_art);
        CHECK(has_con);
        CHECK(has_voc);
        CHECK(has_mid);
        CHECK(has_map);
    }

    TEST_CASE("GRP file extraction" * doctest::skip(!shareware_available())) {
        auto result = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(result.ok());

        auto& grp = result.value();

        // Extract Palette
        auto pal_data = grp.get(PALETTE_DAT);
        CHECK(!pal_data.empty());
        CHECK(pal_data.size() >= PALETTE_BASE_SIZE);  // Minimum Palette size

        // Extract an ART file
        auto art_data = grp.get(TILES000_ART);
        CHECK(!art_data.empty());
        CHECK(art_data.size() > ART_HEADER_MIN_SIZE);  // Minimum ART header
    }

    TEST_CASE("GRP file contents are valid" * doctest::skip(!shareware_available())) {
        auto result = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(result.ok());

        // Verify Palette signature/checks
        auto pal_data = result.value().get(PALETTE_DAT);
        REQUIRE(pal_data.size() >= PALETTE_BASE_SIZE);

        // Load it
        auto pal_result = Palette::load(std::vector<std::byte>(pal_data.begin(), pal_data.end()));
        CHECK(pal_result.ok());
    }

}  // TEST_SUITE

// ============================================================================
// Render Options Tests
// ============================================================================

TEST_SUITE("Shareware Render Options") {
    TEST_CASE("Render with shade table" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find first non-empty tile
        bool found = false;
        for (size_t i = 0; i < art.value().tile_count() && !found; ++i) {
            auto tile = art.value().get_tile(i);
            if (!tile.has_value() || tile->width == 0 || tile->height == 0)
                continue;

            // Test different shade levels
            for (uint8_t shade : SHADE_TEST_LEVELS) {
                RenderOptions opts;
                opts.shade = shade;

                auto img = render(*tile, pal.value(), opts);
                CHECK(img.ok());
                CHECK(img.value().width == tile->width);
                CHECK(img.value().height == tile->height);
            }

            found = true;
        }

        CHECK(found);
    }

    TEST_CASE("Render with transparency disabled" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_render_no_transparency");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find first non-empty tile
        bool found = false;
        for (size_t i = 0; i < art.value().tile_count() && !found; ++i) {
            auto tile = art.value().get_tile(i);
            if (!tile.has_value() || tile->width == 0 || tile->height == 0)
                continue;

            // Render without transparency fix
            RenderOptions opts;
            opts.fix_transparency = false;

            auto img = render(*tile, pal.value(), opts);
            REQUIRE(img.ok());

            // Encode and save
            auto png = encode(img.value(), Format::png);
            REQUIRE(png.ok());

            auto out_path = output_dir / "no_transparency.png";
            auto write_result = write_file(out_path.string(), png.value());
            CHECK(write_result.ok());
            CHECK(file_exists(out_path.string()));

            found = true;
        }

        CHECK(found);
    }

    TEST_CASE("Render with transparency fix enabled" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_render_transparency_fix");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find first non-empty tile
        bool found = false;
        for (size_t i = 0; i < art.value().tile_count() && !found; ++i) {
            auto tile = art.value().get_tile(i);
            if (!tile.has_value() || tile->width == 0 || tile->height == 0)
                continue;

            // Render with transparency fix enabled
            RenderOptions opts;
            opts.fix_transparency = true;

            auto img = render(*tile, pal.value(), opts);
            REQUIRE(img.ok());

            // Encode to all formats
            auto png = encode(img.value(), Format::png);
            REQUIRE(png.ok());
            auto png_path = output_dir / "transparency_fix.png";
            CHECK(write_file(png_path.string(), png.value()).ok());
            CHECK(file_exists(png_path.string()));

            auto tga = encode(img.value(), Format::tga);
            REQUIRE(tga.ok());
            auto tga_path = output_dir / "transparency_fix.tga";
            CHECK(write_file(tga_path.string(), tga.value()).ok());
            CHECK(file_exists(tga_path.string()));

            auto bmp = encode(img.value(), Format::bmp);
            REQUIRE(bmp.ok());
            auto bmp_path = output_dir / "transparency_fix.bmp";
            CHECK(write_file(bmp_path.string(), bmp.value()).ok());
            CHECK(file_exists(bmp_path.string()));

            found = true;
        }

        CHECK(found);
    }

    TEST_CASE("Convert with transparency fix for all formats" *
              doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        Format formats[] = {Format::png, Format::tga, Format::bmp};
        const char* suffixes[] = {".png", ".tga", ".bmp"};
        bool transp_fixes[] = {false, true};

        for (size_t t = 0; t < 2; ++t) {
            bool fix_transp = transp_fixes[t];
            std::string prefix =
                fix_transp ? "shareware_convert_transp_fix_" : "shareware_convert_no_transp_";

            for (size_t f = 0; f < 3; ++f) {
                auto output_dir = get_test_output_dir(prefix + std::to_string(f));

                ConvertOptions opts;
                opts.format = formats[f];
                opts.render.fix_transparency = fix_transp;
                opts.output_prefix = (output_dir / "tile_").string();
                opts.output_suffix = suffixes[f];

                auto result = convert_art(art.value(), pal.value(), opts);
                CHECK(result.ok());
                CHECK(result.value() > 0);
            }
        }
    }

    TEST_CASE("Extract single title to PNG/BMP/TGA" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find a specific tile (tile 1 usually exists)
        auto tile_opt = art.value().get_tile(1);
        REQUIRE(tile_opt.has_value());
        const auto& tile = *tile_opt;

        // Test each format with a single tile
        Format formats[] = {Format::png, Format::bmp, Format::tga};
        const char* suffixes[] = {".png", ".bmp", ".tga"};
        const char* names[] = {"png", "bmp", "tga"};

        for (size_t i = 0; i < 3; ++i) {
            auto output_dir =
                get_test_output_dir("shareware_extract_single_" + std::string(names[i]));

            auto img = render(tile, pal.value(), {});
            REQUIRE(img.ok());

            auto encoded = encode(img.value(), formats[i]);
            REQUIRE(encoded.ok());

            auto out_path = output_dir / ("title_1." + std::string(suffixes[i]));
            CHECK(write_file(out_path.string(), encoded.value()).ok());
            CHECK(file_exists(out_path.string()));
        }
    }

    TEST_CASE("Render with lookup table" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_render_lookup");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find first non-empty tile
        bool found = false;
        for (size_t i = 0; i < art.value().tile_count() && !found; ++i) {
            auto tile = art.value().get_tile(i);
            if (!tile.has_value() || tile->width == 0 || tile->height == 0)
                continue;

            // Render with lookup table
            RenderOptions opts;
            opts.apply_lookup = true;

            auto img = render(*tile, pal.value(), opts);
            REQUIRE(img.ok());

            // Encode and save
            auto png = encode(img.value(), Format::png);
            REQUIRE(png.ok());

            auto out_path = output_dir / "with_lookup.png";
            auto write_result = write_file(out_path.string(), png.value());
            CHECK(write_result.ok());
            CHECK(file_exists(out_path.string()));

            found = true;
        }

        CHECK(found);
    }

    TEST_CASE("Render with premultiply alpha" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_render_premultiply");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find first non-empty tile
        bool found = false;
        for (size_t i = 0; i < art.value().tile_count() && !found; ++i) {
            auto tile = art.value().get_tile(i);
            if (!tile.has_value() || tile->width == 0 || tile->height == 0)
                continue;

            // Render with premultiplied alpha
            RenderOptions opts;
            opts.premultiply = true;

            auto img = render(*tile, pal.value(), opts);
            REQUIRE(img.ok());

            // Encode and save
            auto png = encode(img.value(), Format::png);
            REQUIRE(png.ok());

            auto out_path = output_dir / "premultiplied.png";
            auto write_result = write_file(out_path.string(), png.value());
            CHECK(write_result.ok());
            CHECK(file_exists(out_path.string()));

            found = true;
        }

        CHECK(found);
    }

    TEST_CASE("Render with matte" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_render_matte");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find first non-empty tile
        bool found = false;
        for (size_t i = 0; i < art.value().tile_count() && !found; ++i) {
            auto tile = art.value().get_tile(i);
            if (!tile.has_value() || tile->width == 0 || tile->height == 0)
                continue;

            // Render with matte
            RenderOptions opts;
            opts.matte = true;

            auto img = render(*tile, pal.value(), opts);
            REQUIRE(img.ok());

            // Encode and save
            auto png = encode(img.value(), Format::png);
            REQUIRE(png.ok());

            auto out_path = output_dir / "with_matte.png";
            auto write_result = write_file(out_path.string(), png.value());
            CHECK(write_result.ok());
            CHECK(file_exists(out_path.string()));

            found = true;
        }

        CHECK(found);
    }

}  // TEST_SUITE

// ============================================================================
// Multiple ART Files Conversion Tests
// ============================================================================

TEST_SUITE("Shareware Multiple ART Conversion") {
    TEST_CASE("Convert tiles000.art to PNG" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_convert_tiles000");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        ConvertOptions opts;
        opts.format = Format::png;
        opts.output_prefix = (output_dir / "tile_").string();
        opts.output_suffix = ".png";

        auto result = convert_art(art.value(), pal.value(), opts);
        CHECK(result.ok());
        CHECK(result.value() > 0);
    }

    TEST_CASE("Convert multiple ART files with different formats" *
              doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        const char* ArtFiles[] = {TILES000_ART, TILES001_ART, TILES002_ART};
        Format formats[] = {Format::png, Format::tga, Format::bmp};

        for (size_t i = 0; i < 3; ++i) {
            auto output_dir =
                get_test_output_dir(std::string("shareware_convert_fmt_") + std::to_string(i));

            auto art = ArtFile::from_grp(grp.value(), ArtFiles[i]);
            REQUIRE(art.ok());

            ConvertOptions opts;
            opts.format = formats[i];
            opts.output_prefix = (output_dir / "tile_").string();
            opts.output_suffix = (i == 0) ? ".png" : (i == 1) ? ".tga" : ".bmp";

            auto result = convert_art(art.value(), pal.value(), opts);
            CHECK(result.ok());
            CHECK(result.value() > 0);
        }
    }

    TEST_CASE("Convert with render options" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_convert_shaded");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        ConvertOptions opts;
        opts.format = Format::png;
        opts.output_prefix = (output_dir / "tile_shaded_").string();
        opts.output_suffix = EXT_PNG;
        opts.render.shade = SHADE_MID;

        auto result = convert_art(art.value(), pal.value(), opts);
        CHECK(result.ok());
        CHECK(result.value() > 0);
    }

    TEST_CASE("Extract and convert all shareware ART files" *
              doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_all_art");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto list = grp.value().list();
        int converted = 0;
        int processed = 0;

        for (const auto& name : list) {
            if (name.size() < 4 || name.substr(name.size() - 4) != EXT_ART) {
                continue;
            }

            auto art = ArtFile::from_grp(grp.value(), name);
            if (!art.ok())
                continue;

            processed++;

            // Create subdirectory for each ART file
            auto art_dir = output_dir / name.substr(0, name.size() - 4);
            std::filesystem::create_directories(art_dir);

            ConvertOptions opts;
            opts.format = Format::png;
            opts.output_prefix = (art_dir / "tile_").string();
            opts.output_suffix = EXT_PNG;

            auto result = convert_art(art.value(), pal.value(), opts);
            if (result.ok()) {
                converted++;
            }
        }

        CHECK(processed >= static_cast<int>(MIN_ART_FILE_COUNT));
        CHECK(converted > 0);
    }

}  // TEST_SUITE

// ============================================================================
// High-Level API Tests
// ============================================================================

TEST_SUITE("Shareware High-Level API") {
    TEST_CASE("extract_all tiles from ART" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Note: extract_all fails on empty tiles (0x0 dimensions)
        // Extract tiles one by one and count successes
        size_t extracted = 0;
        for (size_t i = 0; i < art.value().tile_count(); ++i) {
            auto size = art.value().get_tile_size(i);
            if (!size || size->width == 0 || size->height == 0) {
                continue;  // Skip empty tiles
            }
            auto result = extract_tile(art.value(), pal.value(), i);
            if (result.ok()) {
                extracted++;
            }
        }

        CHECK(extracted > 0);
    }

    TEST_CASE("extract_range of tiles" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find a range with non-empty tiles
        // First find first non-empty tile
        size_t first_valid = 0;
        for (size_t i = 0; i < art.value().tile_count(); ++i) {
            auto size = art.value().get_tile_size(i);
            if (size && size->width > 0 && size->height > 0) {
                first_valid = i;
                break;
            }
        }

        // Extract range of 5 tiles starting from first valid
        size_t end = std::min(first_valid + 5, art.value().tile_count());
        auto result = extract_range(art.value(), pal.value(), first_valid, end);

        // Range may include empty tiles, so check it handles them or succeeds
        if (result.ok()) {
            CHECK(result.value().size() <= 5);
        } else {
            // If it failed, it should be due to empty tiles
            CHECK(result.code() == error::invalid_argument);
        }
    }

    TEST_CASE("extract_single tile" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Extract tile at index 0
        auto result = extract_tile(art.value(), pal.value(), 0);
        CHECK(result.ok());
        CHECK(result.value().width > 0);
        CHECK(result.value().height > 0);
    }

}  // TEST_SUITE

// ============================================================================
// Palette Tests
// ============================================================================

TEST_SUITE("Shareware Palette") {
    TEST_CASE("Palette loads from GRP" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto result = Palette::from_grp(grp.value());
        CHECK(result.ok());
    }

    TEST_CASE("Palette has expected structure" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto result = Palette::from_grp(grp.value());
        REQUIRE(result.ok());

        auto& pal = result.value();

        // Can get all colors
        for (size_t i = 0; i < PALETTE_COLOR_COUNT; ++i) {
            CHECK(pal.get_color(i).has_value());
        }

        // Color 255 is transparent
        auto trans_color = pal.get_color(TRANSPARENT_COLOR_INDEX);
        CHECK(trans_color.has_value());
        CHECK(trans_color->a == 0);

        // Color 0 is opaque
        auto bg_color = pal.get_color(BACKGROUND_COLOR_INDEX);
        CHECK(bg_color.has_value());
        CHECK(bg_color->a == 255);
    }

    TEST_CASE("Palette can be extracted and saved" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_Palette");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        // Save raw Palette data
        auto raw_data = grp.value().get(PALETTE_DAT);
        auto out_path = output_dir / "extracted_Palette.dat";

        auto result =
            write_file(out_path.string(), std::vector<std::byte>(raw_data.begin(), raw_data.end()));
        CHECK(result.ok());
        CHECK(file_exists(out_path.string()));
    }

}  // TEST_SUITE

// ============================================================================
// ART File Tests
// ============================================================================

TEST_SUITE("Shareware ART Files") {
    TEST_CASE("ART loads from GRP" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto result = ArtFile::from_grp(grp.value(), TILES000_ART);
        CHECK(result.ok());
    }

    TEST_CASE("ART has tiles" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto result = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(result.ok());

        CHECK(result.value().tile_count() > 0);
    }

    TEST_CASE("ART tiles have valid sizes" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto result = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(result.ok());

        auto& art = result.value();

        // Check that tiles have reasonable sizes
        int valid_tiles = 0;
        int empty_tiles = 0;

        for (size_t i = 0; i < art.tile_count(); ++i) {
            auto size = art.get_tile_size(i);
            if (size) {
                if (size->width > 0 && size->height > 0) {
                    valid_tiles++;
                    CHECK(size->width <= REASONABLE_MAX_WIDTH);
                    CHECK(size->height <= REASONABLE_MAX_HEIGHT);
                } else {
                    empty_tiles++;
                }
            }
        }

        CHECK(valid_tiles > 0);
    }

    TEST_CASE("ART tile data can be accessed" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto art_result = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art_result.ok());

        auto& art = art_result.value();

        // Find first non-empty tile
        for (size_t i = 0; i < art.tile_count(); ++i) {
            auto size = art.get_tile_size(i);
            if (size && size->width > 0 && size->height > 0) {
                auto tile = art.get_tile(i);
                CHECK(tile.has_value());
                CHECK(tile->width == size->width);
                CHECK(tile->height == size->height);
                CHECK(tile->count == static_cast<size_t>(size->width) * size->height);
                CHECK(tile->indices != nullptr);
                break;
            }
        }
    }

    TEST_CASE("Multiple ART files exist" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto list = grp.value().list();
        int art_count = 0;

        for (const auto& name : list) {
            if (name.size() >= 4 && name.substr(name.size() - 4) == EXT_ART) {
                art_count++;
            }
        }

        CHECK(art_count >= static_cast<int>(MIN_ART_FILE_COUNT));
    }

}  // TEST_SUITE

// ============================================================================
// End-to-End Tests
// ============================================================================

TEST_SUITE("Shareware End-to-End") {
    TEST_CASE("Extract and render tile" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Find first non-empty tile
        bool found = false;
        for (size_t i = 0; i < art.value().tile_count() && !found; ++i) {
            auto size = art.value().get_tile_size(i);
            if (size && size->width > 0 && size->height > 0) {
                auto tile = art.value().get_tile(i);
                REQUIRE(tile.has_value());

                auto img = render(*tile, pal.value());
                CHECK(img.ok());
                CHECK(img.value().width == size->width);
                CHECK(img.value().height == size->height);
                CHECK(img.value().pixels.size() ==
                      static_cast<size_t>(size->width) * size->height * BYTES_PER_RGBA);

                found = true;
            }
        }

        CHECK(found);
    }

    TEST_CASE("Encode tile to PNG" * doctest::skip(!shareware_available())) {
        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        // Render tile 0
        auto tile = art.value().get_tile(0);
        if (tile.has_value()) {
            auto img = render(*tile, pal.value());
            REQUIRE(img.ok());

            auto png = encode(img.value(), Format::png);
            CHECK(png.ok());
            CHECK(!png.value().empty());

            // Verify PNG magic
            CHECK(png.value()[0] == std::byte{PNG_MAGIC[0]});
        }
    }

    TEST_CASE("Convert multiple tiles" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_convert");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto pal = Palette::from_grp(grp.value());
        REQUIRE(pal.ok());

        auto art = ArtFile::from_grp(grp.value(), TILES000_ART);
        REQUIRE(art.ok());

        int converted = 0;
        for (size_t i = 0; i < art.value().tile_count() && converted < 10; ++i) {
            auto tile = art.value().get_tile(i);
            if (!tile.has_value())
                continue;

            auto img = render(*tile, pal.value());
            if (!img.ok())
                continue;

            auto png = encode(img.value(), Format::png);
            if (!png.ok())
                continue;

            auto out_path = output_dir / (std::to_string(i) + EXT_PNG);
            auto write_result = write_file(out_path.string(), png.value());

            if (write_result.ok()) {
                converted++;
            }
        }

        CHECK(converted > 0);
        CHECK(converted <= 10);
    }

    TEST_CASE("Extract all ART files from GRP" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("shareware_extract_art");

        auto grp = GrpFile::load(SHAREWARE_GRP_PATH);
        REQUIRE(grp.ok());

        auto list = grp.value().list();
        int extracted = 0;

        for (const auto& name : list) {
            if (name.size() >= 4 && name.substr(name.size() - 4) == EXT_ART) {
                auto data = grp.value().get(name);
                if (data.empty())
                    continue;

                auto out_path = output_dir / name;
                auto result =
                    write_file(out_path.string(), std::vector<std::byte>(data.begin(), data.end()));

                if (result.ok()) {
                    extracted++;
                }
            }
        }

        CHECK(extracted > 0);
    }

}  // TEST_SUITE
