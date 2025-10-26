#include <sstream>

#include <doctest/doctest.h>

#include <art2img/core/error.hpp>

TEST_SUITE("error handling") {
  TEST_CASE("errc enum values are correct") {
    using art2img::core::errc;

    CHECK(static_cast<int>(errc::none) == 0);
    CHECK(static_cast<int>(errc::io_failure) == 1);
    CHECK(static_cast<int>(errc::invalid_art) == 2);
    CHECK(static_cast<int>(errc::invalid_palette) == 3);
    CHECK(static_cast<int>(errc::conversion_failure) == 4);
    CHECK(static_cast<int>(errc::encoding_failure) == 5);
    CHECK(static_cast<int>(errc::unsupported) == 6);
  }

  TEST_CASE("Error struct construction") {
    using art2img::core::errc;
    using art2img::core::Error;

    // Test default construction
    Error default_error;
    CHECK(default_error.message.empty());

    // Test construction with errc and message
    Error art_error(errc::invalid_art, "Invalid ART file");
    CHECK(art_error.code.value() == static_cast<int>(errc::invalid_art));
    CHECK(art_error.message == "Invalid ART file");

    // Test construction with error_code and message
    std::error_code ec =
        std::make_error_code(std::errc::no_such_file_or_directory);
    Error file_error(ec, "File not found");
    CHECK(file_error.code == ec);
    CHECK(file_error.message == "File not found");
  }

  TEST_CASE("make_error_code function") {
    using art2img::core::errc;
    using art2img::core::make_error_code;

    auto ec = make_error_code(errc::invalid_palette);
    CHECK(ec.value() == static_cast<int>(errc::invalid_palette));
    CHECK(ec.category().name() == std::string("art2img"));
  }

  TEST_CASE("Error category name and messages") {
    using art2img::core::errc;
    using art2img::core::make_error_code;

    const auto& category = make_error_code(errc::none).category();
    CHECK(std::string(category.name()) == "art2img");

    CHECK(category.message(static_cast<int>(errc::none)) == "No error");
    CHECK(category.message(static_cast<int>(errc::io_failure)) ==
          "Input/output operation failed");
    CHECK(category.message(static_cast<int>(errc::invalid_art)) ==
          "Invalid or corrupted ART file format");
    CHECK(category.message(static_cast<int>(errc::invalid_palette)) ==
          "Invalid or corrupted palette file format");
    CHECK(category.message(static_cast<int>(errc::conversion_failure)) ==
          "Color conversion or pixel transformation failed");
    CHECK(category.message(static_cast<int>(errc::encoding_failure)) ==
          "Image encoding operation failed");
    CHECK(category.message(static_cast<int>(errc::unsupported)) ==
          "Requested operation or format is not supported");

    // Test unknown error code
    CHECK(category.message(999) == "Unknown error");
  }

  TEST_CASE("make_error_expected functions") {
    using art2img::core::errc;
    using art2img::core::Error;
    using art2img::core::make_error_expected;

    // Test with errc and message
    auto result1 = make_error_expected(errc::invalid_art, "Bad ART file");
    REQUIRE(!result1.has_value());
    CHECK(result1.error().code.value() == static_cast<int>(errc::invalid_art));
    CHECK(result1.error().message == "Bad ART file");

    // Test with error_code and message
    std::error_code ec = std::make_error_code(std::errc::permission_denied);
    auto result2 = make_error_expected(ec, "Access denied");
    REQUIRE(!result2.has_value());
    CHECK(result2.error().code == ec);
    CHECK(result2.error().message == "Access denied");

    // Test with Error object
    Error custom_error(errc::conversion_failure, "Color conversion failed");
    auto result3 = make_error_expected(custom_error);
    REQUIRE(!result3.has_value());
    CHECK(result3.error().code.value() ==
          static_cast<int>(errc::conversion_failure));
    CHECK(result3.error().message == "Color conversion failed");

    // Test with explicit type
    auto result4 = make_error_expected<int>(errc::io_failure, "Read failed");
    REQUIRE(!result4.has_value());
    CHECK(result4.error().code.value() == static_cast<int>(errc::io_failure));
  }

  TEST_CASE("make_success functions") {
    using art2img::core::make_success;

    // Test monostate success
    auto result1 = make_success();
    REQUIRE(result1.has_value());

    // Test success with value
    auto result2 = make_success(42);
    REQUIRE(result2.has_value());
    CHECK(result2.value() == 42);

    // Test success with string
    auto result3 = make_success(std::string("hello"));
    REQUIRE(result3.has_value());
    CHECK(result3.value() == "hello");
  }

  TEST_CASE("std::error_code compatibility") {
    using art2img::core::errc;
    using art2img::core::make_error_code;

    // Test that errc can be implicitly converted to std::error_code
    std::error_code ec = art2img::core::errc::invalid_art;
    CHECK(ec.value() == static_cast<int>(errc::invalid_art));
    CHECK(ec.category().name() == std::string("art2img"));

    // Test error code comparison
    CHECK(ec == make_error_code(errc::invalid_art));
    CHECK(ec != make_error_code(errc::invalid_palette));
  }
}