#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>

// =============================================================================
// Includes and Configuration
// =============================================================================

#include "art_file.hpp"
#include "colors.hpp"
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
        "Convert ART files to PNG, TGA, or BMP with transparency support.\n"
        "GPL v2 License - See LICENSE file for complete terms."};
    app.set_version_flag("-v,--version", "art2img " ART2IMG_VERSION);

    // Show help by default if no arguments provided
    if (argc == 1) {
      std::cout << app.help();
      return 0;
    }

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

    app.add_flag("-F,--fix-transparency,!--no-fix-transparency", cli_options.fix_transparency,
                 "Enable magenta transparency fix (default: enabled)");
    app.add_flag("-q,--quiet", cli_options.quiet, "Suppress all non-essential output");
    app.add_flag("-n,--no-anim", cli_options.no_anim, "Skip animation data generation");
    app.add_flag("-m,--merge-anim", cli_options.merge_anim,
                 "Merge all animation data into a single file (directory mode)");

    // Note: Conflicting flags are automatically handled by the !--no-flag syntax

    // Add footer with usage examples
    app.footer(
        "\nExamples:\n"
        "  art2img tiles.art                  # Convert single ART file\n"
        "  art2img tiles.art -f tga -o out/   # Convert to TGA with output dir\n"
        "  art2img art/ -o images/            # Convert all ART files\n"
        "  art2img tiles.art -p custom.pal    # Use custom palette\n"
        "  art2img tiles.art --no-fix-transparency  # Disable transparency\n"
        "  art2img art/ -m -o game/           # Merge animation data\n"
        "\nFor modders: Use -F for transparency and -m for animation data.");

    // Show help by default if no arguments provided
    if (argc == 1) {
      std::cout << app.help();
      return 0;
    }

    CLI11_PARSE(app, argc, argv);

    // Print banner if not quiet
    if (!cli_options.quiet) {
      art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
      std::cout << "art2img v" << ART2IMG_VERSION << " - Duke Nukem 3D ART File Converter"
                << std::endl;
      std::cout << "=============================================" << std::endl;
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
    art2img::ColorGuard red(art2img::ColorOutput::RED, std::cerr);
    std::cerr << "Error: ";
    std::cerr << e.what() << std::endl;
  } catch (const std::exception& e) {
    art2img::ColorGuard red(art2img::ColorOutput::RED, std::cerr);
    std::cerr << "Error: ";
    std::cerr << e.what() << std::endl;
  } catch (...) {
    art2img::ColorGuard red(art2img::ColorOutput::RED, std::cerr);
    std::cerr << "Error: Unknown exception occurred";
    std::cerr << std::endl;
  }

  art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW, std::cerr);
  std::cerr << "For help, run: art2img --help" << std::endl;
  return 1;
}