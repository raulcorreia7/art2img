#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>

// =============================================================================
// Includes and Configuration
// =============================================================================

#include "art2img/art_file.hpp"
#include "cli_app_builder.hpp"
#include "art2img/colors.hpp"
#include "config.hpp"
#include "art2img/exceptions.hpp"
#include "art2img/extractor_api.hpp"
#include "art2img/palette.hpp"
#include "processor.hpp"
#include "art2img/version.hpp"

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

    auto translation_result = translate_to_processing_options(cli_options);
    if (!translation_result.success()) {
      art2img::ColorGuard red(art2img::ColorOutput::RED, std::cerr);
      if (translation_result.error.has_value()) {
        const auto& error = *translation_result.error;
        const char* code_label = nullptr;
        switch (error.code) {
        case OptionTranslationErrorCode::InvalidFormat:
          code_label = "invalid-format";
          break;
        case OptionTranslationErrorCode::AnimationConflict:
          code_label = "animation-conflict";
          break;
        case OptionTranslationErrorCode::PaletteConflict:
          code_label = "palette-conflict";
          break;
        case OptionTranslationErrorCode::None:
          break;
        }

        std::cerr << "Invalid option combination";
        if (code_label != nullptr) {
          std::cerr << " [" << code_label << "]";
        }
        std::cerr << ": " << error.message << std::endl;
      } else {
        std::cerr << "Invalid option combination" << std::endl;
      }

      art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW, std::cerr);
      std::cerr << "For help, run: art2img --help" << std::endl;
      return 1;
    }

    const ProcessingOptions& processing_options = *translation_result.options;

    // Print banner if not quiet
    if (processing_options.verbose) {
      art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
      std::cout << "art2img v" << ART2IMG_VERSION << " - Duke Nukem 3D ART File Converter"
                << std::endl;
      std::cout << "=============================================" << std::endl;
      std::cout << std::endl;
    }

    // Process based on input type
    CliProcessResult processing_result;
    bool is_directory = std::filesystem::is_directory(cli_options.input_path);

    if (is_directory) {
      processing_result = process_art_directory(cli_options, processing_options);
    } else {
      processing_result = process_single_art_file_wrapper(cli_options, processing_options);
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
