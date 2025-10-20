#include <art2img/export.hpp>
#include <art2img/convert.hpp>
#include <art2img/encode.hpp>
#include <art2img/io.hpp>
#include <filesystem>
#include <string>
#include <system_error>
#include <thread>

namespace art2img {

namespace {

/// @brief Get file extension for the given image format
std::string get_extension(ImageFormat format) {
    switch (format) {
        case ImageFormat::png: return "png";
        case ImageFormat::tga: return "tga";
        case ImageFormat::bmp: return "bmp";
        default: return "bin";
    }
}

/// @brief Generate output path for a tile
std::filesystem::path generate_output_path(
    const std::string& base_name,
    std::size_t tile_index,
    const ExportOptions& options) {

    std::filesystem::path output_path = options.output_dir;

    if (options.organize_by_format) {
        output_path /= get_extension(options.format);
    }

    if (options.organize_by_art_file) {
        output_path /= base_name;
    }

    // Create directories if needed
    std::error_code ec;
    std::filesystem::create_directories(output_path, ec);
    if (ec) {
        // Directory creation failure will be handled by write_binary_file
    }

    const std::string filename = options.filename_prefix + "_" +
                                std::to_string(tile_index) + "." +
                                get_extension(options.format);

    return output_path / filename;
}

/// @brief Export a single tile with the given options
std::expected<std::filesystem::path, Error> export_single_tile_internal(
    const TileView& tile,
    const Palette& palette,
    const std::string& base_name,
    std::size_t tile_index,
    const ExportOptions& options) {

    // Convert tile to RGBA image
    auto image_result = to_rgba(tile, palette, options.conversion_options);
    if (!image_result) {
        return std::unexpected(image_result.error());
    }
    const auto& image = image_result.value();

    // Create image view
    auto image_view = art2img::image_view(image);

    // Encode to specified format
    auto encode_result = encode_image(image_view, options.format);
    if (!encode_result) {
        return std::unexpected(encode_result.error());
    }

    // Generate output path
    const auto output_path = generate_output_path(base_name, tile_index, options);

    // Write to file
    auto write_result = write_binary_file(output_path, encode_result.value());
    if (!write_result) {
        return std::unexpected(write_result.error());
    }

    return output_path;
}

} // anonymous namespace

std::expected<ExportResult, Error> export_tile(
    const TileView& tile,
    const Palette& palette,
    const ExportOptions& options) {

    if (!tile.is_valid()) {
        return std::unexpected(Error{errc::invalid_art, "Invalid tile provided"});
    }

    auto result = export_single_tile_internal(tile, palette, "tile", 0, options);
    if (!result) {
        return std::unexpected(result.error());
    }

    return ExportResult{
        .total_tiles = 1,
        .exported_tiles = 1,
        .output_files = {result.value()}
    };
}

std::expected<ExportResult, Error> export_art_bundle(
    const ArtData& art_data,
    const Palette& palette,
    const ExportOptions& options) {

    ExportResult result;
    result.total_tiles = art_data.tile_count();

    for (std::size_t i = 0; i < art_data.tile_count(); ++i) {
        auto tile_result = art_data.get_tile(i);
        if (!tile_result) {
            continue; // Skip invalid tiles
        }

        const auto& tile = tile_result.value();
        if (!tile.is_valid()) {
            continue; // Skip invalid tiles
        }

        auto export_result = export_single_tile_internal(tile, palette, "art_bundle", i, options);
        if (export_result) {
            result.output_files.push_back(export_result.value());
            ++result.exported_tiles;
        }
    }

    return result;
}

std::expected<ExportResult, Error> export_art_files(
    const std::vector<std::filesystem::path>& art_files,
    const Palette& palette,
    const ExportOptions& options) {

    ExportResult result;

    for (const auto& art_path : art_files) {
        auto art_result = load_art_bundle(art_path);
        if (!art_result) {
            continue; // Skip files that can't be loaded
        }

        const auto& art_data = art_result.value();
        result.total_tiles += art_data.tile_count();

        std::string base_name = art_path.filename().string();
        // Remove extension if present
        const auto dot_pos = base_name.find_last_of('.');
        if (dot_pos != std::string::npos) {
            base_name = base_name.substr(0, dot_pos);
        }

        for (std::size_t i = 0; i < art_data.tile_count(); ++i) {
            auto tile_result = art_data.get_tile(i);
            if (!tile_result) {
                continue;
            }

            const auto& tile = tile_result.value();
            if (!tile.is_valid()) {
                continue;
            }

            auto export_result = export_single_tile_internal(tile, palette, base_name, i, options);
            if (export_result) {
                result.output_files.push_back(export_result.value());
                ++result.exported_tiles;
            }
        }
    }

    return result;
}

} // namespace art2img