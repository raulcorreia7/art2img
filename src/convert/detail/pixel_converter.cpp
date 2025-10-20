#include <art2img/convert/detail/pixel_converter.hpp>

#include <art2img/color_helpers.hpp>
#include <art2img/palette/detail/palette_color.hpp>

namespace art2img::convert::detail {

color::Color PixelConverter::operator()(types::u8 pixel_index) const noexcept
{
    const types::u8 remapped = remap_index(pixel_index);
    auto color = select_palette_color(remapped);
    color = apply_transparency(color, remapped);
    return options.premultiply_alpha ? color.premultiplied() : color;
}

types::u8 PixelConverter::remap_index(types::u8 index) const noexcept
{
    if (!options.apply_lookup || remap.empty() || index >= remap.size())
    {
        return index;
    }
    return remap[index];
}

color::Color PixelConverter::select_palette_color(types::u8 index) const noexcept
{
    if (palette.shade_table_count > 0)
    {
        return palette_shaded_entry_to_color(palette, options.shade_index, index);
    }
    return palette_entry_to_color(palette, index);
}

color::Color PixelConverter::apply_transparency(color::Color color, types::u8 index) const noexcept
{
    if (options.fix_transparency && (index == 0 && options.premultiply_alpha || color::is_build_engine_magenta(color.r, color.g, color.b)))
    {
        // Set RGB to black for fully transparent pixels to prevent color bleeding
        return color::Color(0, 0, 0, 0);
    }
    return color;
}

} // namespace art2img::convert::detail

