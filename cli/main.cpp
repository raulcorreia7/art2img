#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>

// =============================================================================
// Includes and Configuration
// =============================================================================

// Color codes for terminal output
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"

#include "art_file.hpp"
#include "config.hpp"
#include "exceptions.hpp"
#include "extractor_api.hpp"
#include "palette.hpp"
#include "processor.hpp"
#include "version.hpp"

// =============================================================================
// Main Entry Point
// =============================================================================

int main(int argc, char* argv[]) {
  try {
    CliOptions cli_options;
    CLI::App app{
        "art2img - Duke Nukem 3D ART File Converter\n"
        "==========================================\n"
        "Extract images from Duke Nukem 3D ART files and convert them to modern formats.\n"
        "Supports PNG, TGA, and BMP output with palette handling and transparency fixes.\n"
        "Perfect for game modders and retro graphics enthusiasts."};
    app.set_version_flag("-v,--version", "art2img " ART2IMG_VERSION);

    // Positional argument
    app.add_option("ART_FILE|ART_DIRECTORY", cli_options.input_path,
                   "Input ART file or directory containing ART files")
        ->required();

    // Options
    app.add_option("-o,--output", cli_options.output_dir, "Output directory for converted images")
        ->default_val(".");

    app.add_option("-p,--palette", cli_options.palette_file,
                   "Custom palette file (defaults to built-in Duke Nukem 3D palette)")
        ->type_name("FILE");

    app.add_option("-f,--format", cli_options.format, "Output format: tga, png, or bmp")
        ->default_val("png")
        ->check(CLI::IsMember({"tga", "png", "bmp"}));

    auto* fix_flag = app.add_flag("-F,--fix-transparency", cli_options.fix_transparency,
                                  "Enable magenta transparency fix (default: enabled)");
    auto* no_fix_flag =
        app.add_flag("-N,--no-fix-transparency", "Disable magenta transparency fix");
    app.add_flag("-q,--quiet", cli_options.quiet, "Suppress all non-essential output");
    app.add_flag("-n,--no-anim", cli_options.no_anim, "Skip animation data generation");
    app.add_flag("-m,--merge-anim", cli_options.merge_anim,
                 "Merge all animation data into a single file (directory mode)");

    // Configure conflicting flags
    fix_flag->excludes(no_fix_flag);
    no_fix_flag->excludes(fix_flag);

    // Add footer with usage examples
    app.footer(
        "\nExamples:\n"
        "  art2img tiles.art                           # Convert single ART file to PNG\n"
        "  art2img tiles.art -f tga -o output/         # Convert to TGA with custom output\n"
        "  art2img art/ -o images/                     # Convert all ART files in directory\n"
        "  art2img tiles.art -p custom.pal             # Use custom palette file\n"
        "  art2img tiles.art -N                        # Disable transparency fix\n"
        "  art2img art/ -m -o game/                    # Merge all animation data\n"
        "\nFor Duke Nukem 3D modders: Use the -F flag for proper transparency handling\n"
        "in sprites, and the -m flag to merge all animation data when processing\n"
        "multiple ART files.");

    CLI11_PARSE(app, argc, argv);

    // Print banner if not quiet
    if (!cli_options.quiet) {
      std::cout << COLOR_CYAN << "art2img v" << ART2IMG_VERSION
                << " - Duke Nukem 3D ART File Converter" << COLOR_RESET << std::endl;
      std::cout << COLOR_CYAN << "=============================================" << COLOR_RESET
                << std::endl;
      std::cout << std::endl;
    }

    // Process based on input type
    bool success = false;
    bool is_directory = std::filesystem::is_directory(cli_options.input_path);

    if (is_directory) {
      success = process_art_directory(cli_options);
    } else {
      success = process_single_art_file_wrapper(cli_options);
    }

    return success ? 0 : 1;

  } catch (const art2img::ArtException& e) {
    std::cerr << COLOR_RED << "Error: " << COLOR_RESET << e.what() << std::endl;
    std::cerr << COLOR_YELLOW << "For help, run: art2img --help" << COLOR_RESET << std::endl;
    return 1;
  } catch (const std::exception& e) {
    std::cerr << COLOR_RED << "Error: " << COLOR_RESET << e.what() << std::endl;
    std::cerr << COLOR_YELLOW << "For help, run: art2img --help" << COLOR_RESET << std::endl;
    return 1;
  } catch (...) {
    std::cerr << COLOR_RED << "Error: Unknown exception occurred" << COLOR_RESET << std::endl;
    std::cerr << COLOR_YELLOW << "For help, run: art2img --help" << COLOR_RESET << std::endl;
    return 1;
  }
}