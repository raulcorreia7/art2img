#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>

// =============================================================================
// Includes and Configuration
// =============================================================================

#include "art_file.hpp"
#include "cli_app_builder.hpp"
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
    art2img::CliAppBuilder builder;
    auto app = builder.build(cli_options);

    // Show help by default if no arguments provided
    if (argc == 1) {
      std::cout << app->help();
      return 0;
    }

    CLI11_PARSE(*app, argc, argv);

    // Print banner if not quiet
    if (!cli_options.quiet) {
      art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
      std::cout << "art2img v" << ART2IMG_VERSION
                << " - Duke Nukem 3D ART File Converter" << std::endl;
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
