#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <expected>
#include <vector>
#include "art2img/core/error.hpp"

namespace art2img {

using Byte = std::byte;
using ByteSpan = std::span<const Byte>;

struct Error {
    std::string message;
    
    Error(std::string m) : message(std::move(m)) {}
    Error(const char* m) : message(m) {}
    Error(const core::Error& e) : message(e.message) {}
    Error(core::errc, std::string m) : message(std::move(m)) {}
};

template <typename T>
using Result = std::expected<T, Error>;

} // namespace art2img
