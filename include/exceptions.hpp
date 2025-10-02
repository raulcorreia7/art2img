#pragma once

#include <stdexcept>
#include <string>

namespace art2img {

class ArtException : public std::runtime_error {
public:
    explicit ArtException(const std::string& message) : std::runtime_error(message) {}
};

} // namespace art2img