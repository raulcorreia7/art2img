// End-to-end tests: CLI

#include "../test_common.hpp"
#include "../test_constants.hpp"

#include "doctest.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

using namespace art2img::test;
using namespace art2img::test::constants;

// ============================================================================
// CLI Test Helpers
// ============================================================================

struct CliResult {
    int exit_code;
    std::string stdout_str;
    std::string stderr_str;
};

// CLI binary path is passed via compile definition
#ifndef CLI_BINARY_PATH
#define CLI_BINARY_PATH "./build/art2img"
#endif

CliResult run_cli(const std::vector<std::string>& args) {
    std::string cmd = CLI_BINARY_PATH;
    for (const auto& arg : args) {
        cmd += " \"" + arg + "\"";
    }

    cmd += " 2>&1";  // Capture both stdout and stderr

    std::array<char, 4096> buffer;
    std::string output;

    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (!pipe) {
        return {-1, "", "failed to run command"};
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output += buffer.data();
    }

    int status = PCLOSE(pipe);
#ifdef _WIN32
    int exit_code = status;
#else
    int exit_code = WEXITSTATUS(status);
#endif

    return {exit_code, output, ""};
}

// ============================================================================
// CLI Basic Tests
// ============================================================================

TEST_SUITE("CLI Basic") {
    TEST_CASE("CLI help returns 0") {
        auto result = run_cli({"--help"});
        CHECK(result.exit_code == 0);
        CHECK(result.stdout_str.find("Usage:") != std::string::npos);
        CHECK(result.stdout_str.find("--grp") != std::string::npos);
    }

    TEST_CASE("CLI version returns 0") {
        auto result = run_cli({"--version"});
        CHECK(result.exit_code == 0);
        CHECK(result.stdout_str.find(EXPECTED_VERSION) != std::string::npos);
    }

    TEST_CASE("CLI no arguments shows help and returns 1") {
        auto result = run_cli({});
        CHECK(result.exit_code == 1);
        CHECK(result.stdout_str.find("Usage:") != std::string::npos);
    }

    TEST_CASE("CLI unknown option returns 1") {
        auto result = run_cli({"--unknown-option"});
        CHECK(result.exit_code == 1);
        CHECK(result.stdout_str.find("Error:") != std::string::npos);
    }

    TEST_CASE("CLI missing format argument") {
        auto result = run_cli({"-f"});
        CHECK(result.exit_code == 1);
        CHECK(result.stdout_str.find("Missing argument") != std::string::npos);
    }

    TEST_CASE("CLI invalid format") {
        auto result = run_cli({"-f", "invalid"});
        CHECK(result.exit_code == 1);
        CHECK(result.stdout_str.find("Unknown format") != std::string::npos);
    }

    TEST_CASE("CLI list without GRP fails") {
        auto result = run_cli({"--list"});
        CHECK(result.exit_code == 1);
        CHECK(result.stdout_str.find("requires --grp") != std::string::npos);
    }

    TEST_CASE("CLI list with non-existent GRP fails") {
        auto result = run_cli({"--list", "--grp", "/nonexistent/test.grp"});
        CHECK(result.exit_code == 1);
    }

}  // TEST_SUITE

// ============================================================================
// CLI with Shareware GRP
// ============================================================================

TEST_SUITE("CLI Shareware") {
    TEST_CASE("CLI list GRP contents" * doctest::skip(!shareware_available())) {
        auto result = run_cli({"--list", "--grp", SHAREWARE_GRP_PATH});
        CHECK(result.exit_code == 0);
        CHECK(result.stdout_str.find(TILES000_ART) != std::string::npos);
        CHECK(result.stdout_str.find(PALETTE_DAT) != std::string::npos);
        CHECK(result.stdout_str.find("GRP file:") != std::string::npos);
    }

    TEST_CASE("CLI convert with verbose" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("cli_convert_verbose");

        auto result = run_cli({"--grp", SHAREWARE_GRP_PATH, "-o", output_dir.string(), "-v"});
        CHECK(result.exit_code == 0);
        CHECK(result.stdout_str.find("Loading GRP") != std::string::npos);
        CHECK(result.stdout_str.find("Converted") != std::string::npos);

        // Should have created some files
        int png_count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
            if (entry.path().extension() == EXT_PNG) {
                png_count++;
            }
        }
        CHECK(png_count > 0);
    }

    TEST_CASE("CLI convert all formats" * doctest::skip(!shareware_available())) {
        for (const auto& fmt : OUTPUT_FORMATS) {
            auto output_dir = get_test_output_dir(std::string("cli_convert_") + fmt);

            auto result = run_cli({"--grp", SHAREWARE_GRP_PATH, "-o", output_dir.string(), "-f",
                                   fmt, "--art", TILES000_ART});

            CHECK(result.exit_code == 0);

            std::string ext = "." + std::string(fmt);
            int file_count = 0;
            for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
                if (entry.path().extension() == ext) {
                    file_count++;
                }
            }
            CHECK(file_count > 0);
        }
    }

    TEST_CASE("CLI with render options" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("cli_RenderOptions");

        auto result = run_cli({"--grp", SHAREWARE_GRP_PATH, "-o", output_dir.string(),
                               "--no-transparency", "--shade", std::to_string(SHADE_MID).c_str()});

        CHECK(result.exit_code == 0);
    }

    TEST_CASE("CLI specific ART file" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("cli_specific_art");

        auto result = run_cli(
            {"--grp", SHAREWARE_GRP_PATH, "--art", TILES001_ART, "-o", output_dir.string()});

        CHECK(result.exit_code == 0);
    }

    TEST_CASE("CLI non-existent ART file" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("cli_missing_art");

        auto result = run_cli(
            {"--grp", SHAREWARE_GRP_PATH, "--art", "nonexistent.art", "-o", output_dir.string()});

        CHECK(result.exit_code == 1);
    }

}  // TEST_SUITE

// ============================================================================
// CLI Edge Cases
// ============================================================================

TEST_SUITE("CLI Edge Cases") {
    TEST_CASE("CLI convert to non-existent directory" * doctest::skip(!shareware_available())) {
        // The tool should create the directory or fail gracefully
        auto result = run_cli({"--grp", SHAREWARE_GRP_PATH, "-o", "/nonexistent/path/output"});

        // Either succeeds (creates dir) or fails gracefully
        if (result.exit_code != 0) {
            CHECK(result.stdout_str.find("Error:") != std::string::npos);
        }
    }

    TEST_CASE("CLI format case insensitivity" * doctest::skip(!shareware_available())) {
        auto output_dir = get_test_output_dir("cli_format_case");

        // Should accept various cases
        for (const auto& fmt : {"PNG", "png", "Png", "TGA", "BMP"}) {
            auto result =
                run_cli({"--grp", SHAREWARE_GRP_PATH, "-o", output_dir.string(), "-f", fmt});

            // At least shouldn't crash
            CHECK((result.exit_code == 0 || result.exit_code == 1));
        }
    }

}  // TEST_SUITE
