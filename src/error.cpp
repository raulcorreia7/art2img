#include <art2img/error.hpp>

namespace art2img
{

    const char *error_category::name() const noexcept
    {
        return "art2img";
    }

    std::string error_category::message(int ev) const
    {
        switch (static_cast<errc>(ev))
        {
        case errc::none:
            return "No error";
        case errc::io_failure:
            return "Input/output operation failed";
        case errc::invalid_art:
            return "Invalid or corrupted ART file format";
        case errc::invalid_palette:
            return "Invalid or corrupted palette file format";
        case errc::conversion_failure:
            return "Color conversion or pixel transformation failed";
        case errc::encoding_failure:
            return "Image encoding operation failed";
        case errc::unsupported:
            return "Requested operation or format is not supported";
        default:
            return "Unknown error";
        }
    }

    const std::error_category &error_category::instance()
    {
        static const error_category instance;
        return instance;
    }

    std::error_code make_error_code(errc e) noexcept
    {
        return std::error_code(static_cast<int>(e), error_category::instance());
    }

} // namespace art2img