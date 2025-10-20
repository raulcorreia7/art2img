#pragma once

#include <art2img/types.hpp>
#include <art2img/error.hpp>
#include <art2img/convert.hpp>
#include <expected>
#include <variant>
#include <vector>

namespace art2img {



/// @brief PNG encoding options
struct PngOptions {
    /// @brief Compression level (0-9, where 0 is no compression and 9 is maximum)
    u8 compression_level = 6;
    
    /// @brief Use PNG filters (typically improves compression)
    bool use_filters = true;
    
    /// @brief Convert to grayscale if possible
    bool convert_to_grayscale = false;
    
    /// @brief Default constructor
    PngOptions() = default;
    
    /// @brief Constructor with explicit values
    PngOptions(u8 level, bool filters, bool grayscale)
        : compression_level(level), use_filters(filters), convert_to_grayscale(grayscale) {}
};

/// @brief TGA encoding options
struct TgaOptions {
    /// @brief Use RLE compression (true = compressed, false = uncompressed)
    bool use_rle = false;
    
    /// @brief Write TGA with alpha channel (true = 32-bit, false = 24-bit)
    bool include_alpha = true;
    
    /// @brief Flip image vertically (TGA origin is typically bottom-left)
    bool flip_vertically = false;
    
    /// @brief Default constructor
    TgaOptions() = default;
    
    /// @brief Constructor with explicit values
    TgaOptions(bool rle, bool alpha, bool flip)
        : use_rle(rle), include_alpha(alpha), flip_vertically(flip) {}
};

/// @brief BMP encoding options
struct BmpOptions {
    /// @brief Write BMP with alpha channel (true = 32-bit, false = 24-bit)
    bool include_alpha = true;
    
    /// @brief Flip image vertically (BMP origin is bottom-left)
    bool flip_vertically = false;
    
    /// @brief Default constructor
    BmpOptions() = default;
    
    /// @brief Constructor with explicit values
    BmpOptions(bool alpha, bool flip)
        : include_alpha(alpha), flip_vertically(flip) {}
};

/// @brief Variant type for encoding options
/// @note std::monostate represents "use default options"
using EncodeOptions = std::variant<std::monostate, PngOptions, TgaOptions, BmpOptions>;

// ============================================================================
// ENCODING FUNCTIONS
// ============================================================================

/// @brief Encode an image view to PNG format
/// @param image The image view to encode
/// @param options PNG encoding options (optional)
/// @return Expected vector of encoded bytes on success, Error on failure
std::expected<std::vector<byte>, Error> encode_png(
    const ImageView& image, 
    const PngOptions& options = {});

/// @brief Encode an image view to TGA format
/// @param image The image view to encode
/// @param options TGA encoding options (optional)
/// @return Expected vector of encoded bytes on success, Error on failure
std::expected<std::vector<byte>, Error> encode_tga(
    const ImageView& image, 
    const TgaOptions& options = {});

/// @brief Encode an image view to BMP format
/// @param image The image view to encode
/// @param options BMP encoding options (optional)
/// @return Expected vector of encoded bytes on success, Error on failure
std::expected<std::vector<byte>, Error> encode_bmp(
    const ImageView& image, 
    const BmpOptions& options = {});

/// @brief Encode an image view to the specified format
/// @param image The image view to encode
/// @param format The target image format
/// @param options Encoding options (optional, format-specific)
/// @return Expected vector of encoded bytes on success, Error on failure
std::expected<std::vector<byte>, Error> encode_image(
    const ImageView& image, 
    ImageFormat format, 
    EncodeOptions options = {});

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/// @brief Get the default options for a specific image format
/// @param format The image format
/// @return Default options for the format
EncodeOptions get_default_options(ImageFormat format);

/// @brief Validate image view dimensions and stride for encoding
/// @param image The image view to validate
/// @return Expected success on valid image, Error on failure
std::expected<std::monostate, Error> validate_image_for_encoding(const ImageView& image);

/// @brief Get a string representation of the image format
/// @param format The image format
/// @return String name of the format
const char* format_to_string(ImageFormat format);

} // namespace art2img