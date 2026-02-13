// art2img - Convert Build Engine ART files to modern image formats

#include "args.hpp"
#include "ops.hpp"

#include <iostream>

int main(int argc, char** argv) {
  // Parse arguments
  auto cfg_result = art2img_cli::parse_args(argc, argv);
  if (!cfg_result) {
    std::cerr << "Error: " << cfg_result.message() << "\n";
    art2img_cli::print_usage(argv[0]);
    return 1;
  }

  const auto& cfg = cfg_result.value();

  // Dispatch to appropriate command
  switch (cfg.run_mode) {
    case art2img_cli::Mode::kHelp:
      art2img_cli::print_usage(argv[0]);
      return 0;

    case art2img_cli::Mode::kVersion:
      art2img_cli::print_version();
      return 0;

    case art2img_cli::Mode::kList:
      return art2img_cli::cmd_list(cfg);

    case art2img_cli::Mode::kConvert:
      return art2img_cli::cmd_convert(cfg);
  }

  return 0;  // Unreachable
}
