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
    art2img::CliAppBuilder builder;

    // Show help by default if no arguments provided
    if (argc == 1) {
      CliOptions help_options;
      auto help_app = builder.build(help_options);
      std::cout << help_app->help();
      return 0;
    }

    CliOptions cli_options;
    auto app = builder.build(cli_options);

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
    CliProcessResult processing_result;
    bool is_directory = std::filesystem::is_directory(cli_options.input_path);

    if (is_directory) {
      processing_result = process_art_directory(cli_options);
    } else {
      processing_result = process_single_art_file_wrapper(cli_options);
    }

    if (!processing_result.success) {
      art2img::ColorGuard red(art2img::ColorOutput::RED, std::cerr);
      std::cerr << "Error: ";
      if (!processing_result.error_message.empty()) {
        std::cerr << processing_result.error_message << std::endl;
      } else {
        std::cerr << "Processing failed." << std::endl;
      }

      art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW, std::cerr);
      std::cerr << "For help, run: art2img --help" << std::endl;
      return 1;
    }

    return 0;

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
