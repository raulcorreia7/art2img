#include <doctest/doctest.h>
#include <art2img/core/error.hpp>

using namespace art2img::core;

TEST_SUITE("Error Formatting")
{
  TEST_CASE("format_error_message with context")
  {
    const std::string base = "Base error message";
    const std::string context = "Additional context";
    const std::string result = format_error_message(base, context);

    CHECK(result == "Base error message (Additional context)");
  }

  TEST_CASE("format_error_message without context")
  {
    const std::string base = "Base error message";
    const std::string context = "";
    const std::string result = format_error_message(base, context);

    CHECK(result == "Base error message");
  }

  TEST_CASE("format_file_error")
  {
    const std::string base = "File operation failed";
    const std::filesystem::path file_path = "/path/to/file.txt";
    const std::string result = format_file_error(base, file_path);

    CHECK(result == "File operation failed [file: /path/to/file.txt]");
  }

  TEST_CASE("format_tile_error")
  {
    const std::string base = "Tile processing failed";
    const std::size_t tile_index = 42;
    const std::string result = format_tile_error(base, tile_index);

    CHECK(result == "Tile processing failed [tile: 42]");
  }
}