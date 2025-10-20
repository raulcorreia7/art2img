#pragma once

#include <art2img/types.hpp>

#if __has_include(<span>)
#include <span>
#endif

namespace art2img::color {

/// @brief Create a Color from RGBA channel values
constexpr inline Color make_rgba(
    types::u8 red,
    types::u8 green,
    types::u8 blue,
    types::u8 alpha = 255) noexcept
{
    return Color(red, green, blue, alpha);
}

/// @brief Create a Color from BGR-ordered channel values (legacy Build assets)
constexpr inline Color make_from_bgr(
    types::u8 blue,
    types::u8 green,
    types::u8 red,
    types::u8 alpha = 255) noexcept
{
    return Color(red, green, blue, alpha);
}

/// @brief Pack RGBA channel values into a 32-bit RGBA word
constexpr inline types::u32 pack_rgba(
    types::u8 red,
    types::u8 green,
    types::u8 blue,
    types::u8 alpha = 255) noexcept
{
    return make_rgba(red, green, blue, alpha).to_packed(Format::RGBA);
}

/// @brief Unpack a 32-bit RGBA word into a Color
constexpr inline Color unpack_rgba(types::u32 packed) noexcept
{
    return Color(packed, Format::RGBA);
}

/// @brief Store a Color into a raw RGBA byte buffer
inline void write_rgba(types::u8* destination, const Color& color) noexcept
{
    destination[0] = color.r;
    destination[1] = color.g;
    destination[2] = color.b;
    destination[3] = color.a;
}

/// @brief Load a Color from a raw RGBA byte buffer
inline Color read_rgba(const types::u8* source) noexcept
{
    return make_rgba(source[0], source[1], source[2], source[3]);
}

#if __has_include(<span>)
/// @brief Store a Color into a sized RGBA span
inline void write_rgba(std::span<types::u8, art2img::constants::RGBA_CHANNEL_COUNT> destination,
                       const Color& color) noexcept
{
    destination[0] = color.r;
    destination[1] = color.g;
    destination[2] = color.b;
    destination[3] = color.a;
}

/// @brief Load a Color from a sized RGBA span
inline Color read_rgba(std::span<const types::u8, art2img::constants::RGBA_CHANNEL_COUNT> source) noexcept
{
    return make_rgba(source[0], source[1], source[2], source[3]);
}
#endif

/// @brief Check if RGB values match Build Engine magenta (252, 0, 252) with tolerance
constexpr inline bool is_build_engine_magenta(types::u8 r, types::u8 g, types::u8 b) noexcept
{
    return r >= 250u && b >= 250u && g <= 5u;
}

} // namespace art2img::color
