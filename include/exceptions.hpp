#pragma once

#include <stdexcept>
#include <string>

namespace art2img {

class ArtException : public std::runtime_error {
public:
    explicit ArtException(const std::string& message) : std::runtime_error(message) {}
};

class ArtFileException : public ArtException {
public:
    explicit ArtFileException(const std::string& message) : ArtException(message) {}
};

class PaletteException : public ArtException {
public:
    explicit PaletteException(const std::string& message) : ArtException(message) {}
};

class ExtractionException : public ArtException {
public:
    explicit ExtractionException(const std::string& message) : ArtException(message) {}
};

} // namespace art2img