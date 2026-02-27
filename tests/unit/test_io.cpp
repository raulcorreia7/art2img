// Unit tests: File I/O utilities

#include "../test_common.hpp"

// Undef CLI_BINARY_PATH macro if set by build system to avoid conflict with test_constants.hpp
#ifdef CLI_BINARY_PATH
#undef CLI_BINARY_PATH
#endif
#include "../test_constants.hpp"

#include "doctest.h"

#include <art2img.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>

using namespace art2img;
using namespace art2img::test;
using namespace art2img::test::constants;

TEST_SUITE("IO") {
    TEST_CASE("read_file existing file") {
        auto output_dir = get_test_output_dir("io_read_existing");
        auto test_file = output_dir / "test.txt";

        // Create test file
        {
            std::ofstream f(test_file);
            f << "Hello, World!";
        }

        auto result = read_file(test_file.string());
        CHECK(result.ok());
        CHECK(result.value().size() == 13);
        CHECK(std::string(reinterpret_cast<const char*>(result.value().data()), 13) ==
              "Hello, World!");
    }

    TEST_CASE("read_file non-existent file") {
        auto result = read_file("/nonexistent/path/to/file.txt");
        CHECK(!result.ok());
        CHECK(result.code() == error::io_error);
    }

    TEST_CASE("read_file empty file") {
        auto output_dir = get_test_output_dir("io_read_empty");
        auto test_file = output_dir / "empty.txt";

        // Create empty file
        std::ofstream f(test_file);
        f.close();

        auto result = read_file(test_file.string());
        CHECK(result.ok());
        CHECK(result.value().empty());
    }

    TEST_CASE("write_file creates file") {
        auto output_dir = get_test_output_dir("io_write_create");
        auto test_file = output_dir / "output.bin";

        std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
        auto result = write_file(test_file.string(), data);
        CHECK(result.ok());
        CHECK(std::filesystem::exists(test_file));

        // Verify content
        auto read_result = read_file(test_file.string());
        CHECK(read_result.ok());
        CHECK(read_result.value() == data);
    }

    TEST_CASE("write_file overwrites existing") {
        auto output_dir = get_test_output_dir("io_write_overwrite");
        auto test_file = output_dir / "overwrite.bin";

        // Write initial content
        std::vector<std::byte> data1 = {std::byte{0xAA}};
        write_file(test_file.string(), data1);

        // Overwrite with new content
        std::vector<std::byte> data2 = {std::byte{0xBB}, std::byte{0xCC}};
        auto result = write_file(test_file.string(), data2);
        CHECK(result.ok());

        auto read_result = read_file(test_file.string());
        CHECK(read_result.value().size() == 2);
        CHECK(read_result.value()[0] == std::byte{0xBB});
    }

    TEST_CASE("write_file to non-existent directory fails") {
        std::vector<std::byte> data = {std::byte{0x01}};
        auto result = write_file("/nonexistent_dir_xyz/file.bin", data);
        CHECK(!result.ok());
        CHECK(result.code() == error::io_error);
    }

}  // TEST_SUITE
