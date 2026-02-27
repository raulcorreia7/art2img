// Unit tests: GRP file format

#include "../test_common.hpp"

// Undef CLI_BINARY_PATH macro if set by build system to avoid conflict with test_constants.hpp
#ifdef CLI_BINARY_PATH
#undef CLI_BINARY_PATH
#endif
#include "../test_constants.hpp"

#include "doctest.h"

#include <art2img.hpp>

#include <cstring>
#include <vector>

using namespace art2img;
using namespace art2img::test;
using namespace art2img::test::constants;

TEST_SUITE("GRP") {
    TEST_CASE("GrpFile load empty data") {
        std::vector<std::byte> empty;
        auto result = GrpFile::load(empty);
        CHECK(!result.ok());
        CHECK(result.code() == error::invalid_argument);
    }

    TEST_CASE("GrpFile load too small") {
        std::vector<std::byte> small(8);
        auto result = GrpFile::load(small);
        CHECK(!result.ok());
        CHECK(result.code() == error::corrupted_data);
    }

    TEST_CASE("GrpFile load bad signature") {
        std::vector<std::byte> bad_sig(16);
        std::memcpy(bad_sig.data(), "BadSignature", 12);
        auto result = GrpFile::load(bad_sig);
        CHECK(!result.ok());
        CHECK(result.code() == error::invalid_format);
    }

    TEST_CASE("GrpFile load valid") {
        auto data = make_test_grp();
        auto result = GrpFile::load(data);
        CHECK(result.ok());

        auto& grp = result.value();
        CHECK(grp.count() == 2);
        CHECK(grp.has("test.txt"));
        CHECK(grp.has("tiles.art"));
    }

    TEST_CASE("GrpFile case insensitive lookup") {
        auto data = make_test_grp();
        auto result = GrpFile::load(data);
        REQUIRE(result.ok());

        auto& grp = result.value();
        CHECK(grp.has("TEST.TXT"));
        CHECK(grp.has("Test.Txt"));
        CHECK(grp.has("TILES.ART"));
    }

    TEST_CASE("GrpFile get file content") {
        auto data = make_test_grp();
        auto result = GrpFile::load(data);
        REQUIRE(result.ok());

        auto& grp = result.value();
        auto file = grp.get("test.txt");
        CHECK(file.size() == 4);
        CHECK(static_cast<char>(file[0]) == 't');
        CHECK(static_cast<char>(file[1]) == 'e');
        CHECK(static_cast<char>(file[2]) == 's');
        CHECK(static_cast<char>(file[3]) == 't');
    }

    TEST_CASE("GrpFile get non-existent file") {
        auto data = make_test_grp();
        auto result = GrpFile::load(data);
        REQUIRE(result.ok());

        auto& grp = result.value();
        auto file = grp.get("missing.txt");
        CHECK(file.empty());
    }

    TEST_CASE("GrpFile list returns all files") {
        auto data = make_test_grp();
        auto result = GrpFile::load(data);
        REQUIRE(result.ok());

        auto& grp = result.value();
        auto list = grp.list();
        CHECK(list.size() == 2);
        CHECK(list[0] == "test.txt");
        CHECK(list[1] == "tiles.art");
    }

    TEST_CASE("GrpFile load from path") {
        auto output_dir = get_test_output_dir("grp_load_path");
        auto grp_path = output_dir / "test.grp";

        // Create GRP file
        auto data = make_test_grp();
        write_file(grp_path.string(), data);

        auto result = GrpFile::load(grp_path.string());
        CHECK(result.ok());
        CHECK(result.value().count() == 2);
    }

    TEST_CASE("GrpFile load non-existent path") {
        auto result = GrpFile::load("/nonexistent/test.grp");
        CHECK(!result.ok());
        CHECK(result.code() == error::io_error);
    }

    TEST_CASE("GrpFile zero files") {
        std::vector<std::byte> data;
        data.insert(data.end(), reinterpret_cast<const std::byte*>(GRP_SIGNATURE),
                    reinterpret_cast<const std::byte*>(GRP_SIGNATURE + GRP_SIGNATURE_LENGTH));
        // File count: 0
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});

        auto result = GrpFile::load(data);
        CHECK(result.ok());
        CHECK(result.value().count() == 0);
        CHECK(result.value().list().empty());
    }

    TEST_CASE("GrpFile corrupt directory entry") {
        std::vector<std::byte> data;
        data.insert(data.end(), reinterpret_cast<const std::byte*>(GRP_SIGNATURE),
                    reinterpret_cast<const std::byte*>(GRP_SIGNATURE + GRP_SIGNATURE_LENGTH));
        // File count: 1
        data.push_back(std::byte{1});
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
        // Directory entry (truncated)
        char name[12] = "truncated.";
        data.insert(data.end(), reinterpret_cast<std::byte*>(name),
                    reinterpret_cast<std::byte*>(name + 12));
        // Missing size bytes

        auto result = GrpFile::load(data);
        CHECK(!result.ok());
    }

}  // TEST_SUITE
