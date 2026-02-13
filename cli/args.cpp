#include "args.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>

namespace art2img_cli {

void print_usage(const char* prog) {
  std::cerr << "Usage: " << prog << " [options] [input] [Palette]\n"
            << "\n"
            << "Convert Build Engine ART files to images\n"
            << "\n"
            << "Modes:\n"
            << "  --list                List GRP contents and exit\n"
            << "  -h, --help            Show this help\n"
            << "  --version             Show version\n"
            << "\n"
            << "Input Options:\n"
            << "  --grp <file>          Load ART from GRP archive\n"
            << "  --art <name>          ART filename in GRP (default: auto-detect)\n"
            << "  -p, --Palette <file>  Palette file (default: auto-detect from GRP)\n"
            << "\n"
            << "Output Options:\n"
            << "  -o, --output <dir>    Output directory (default: current)\n"
            << "  -f, --format <fmt>    Output format: png, tga, bmp (default: png)\n"
            << "\n"
            << "Rendering Options:\n"
            << "  --shade <n>           Apply shade table index (0-31)\n"
            << "  --lookup              Enable lookup table remapping\n"
            << "  --no-transparency     Disable transparency fix (index 255 opaque)\n"
            << "  --premultiply         Enable alpha premultiplication\n"
            << "  --matte               Enable matte hygiene\n"
            << "\n"
            << "Other:\n"
            << "  -v, --verbose         Verbose output\n"
            << "\n"
            << "Examples:\n"
            << "  " << prog << " --grp DUKE3D.GRP --list\n"
            << "  " << prog << " --grp DUKE3D.GRP -o output/\n"
            << "  " << prog << " TILES000.ART PALETTE.DAT -o output/\n"
            << "  " << prog << " --grp DUKE3D.GRP -f png --matte --verbose\n";
}

void print_version() {
  std::cout << "art2img 2.0.0\n";
}

static art2img::Result<art2img::Format> parse_format(const std::string& s) {
  std::string lowered = s;
  std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);

  if (lowered == "png")
    return art2img::Format::png;
  if (lowered == "tga")
    return art2img::Format::tga;
  if (lowered == "bmp")
    return art2img::Format::bmp;
  return art2img::Error{art2img::error::invalid_argument, "Unknown format: " + s};
}

art2img::Result<Config> parse_args(int argc, char** argv) noexcept {
  Config cfg;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    // Mode switches
    if (arg == "-h" || arg == "--help") {
      cfg.run_mode = Mode::kHelp;
      return cfg;
    } else if (arg == "--version") {
      cfg.run_mode = Mode::kVersion;
      return cfg;
    } else if (arg == "--list") {
      cfg.run_mode = Mode::kList;
    }

    // Input options
    else if (arg == "--grp" && i + 1 < argc) {
      cfg.grp_path = argv[++i];
    } else if (arg == "--art" && i + 1 < argc) {
      cfg.art_name = argv[++i];
    } else if ((arg == "-p" || arg == "--Palette") && i + 1 < argc) {
      cfg.palette_path = argv[++i];
    }

    // Output options
    else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
      cfg.output_dir = argv[++i];
    } else if (arg == "-f" || arg == "--format") {
      if (i + 1 >= argc) {
        return art2img::Error{art2img::error::invalid_argument, "Missing argument for " + arg};
      }
      auto fmt_result = parse_format(argv[++i]);
      if (!fmt_result) {
        return fmt_result.error();
      }
      cfg.fmt = fmt_result.value();
    }

    // Rendering options
    else if (arg == "--shade" && i + 1 < argc) {
      cfg.render.shade = static_cast<uint8_t>(std::atoi(argv[++i]));
    } else if (arg == "--lookup") {
      cfg.render.apply_lookup = true;
    } else if (arg == "--no-transparency") {
      cfg.render.fix_transparency = false;
    } else if (arg == "--premultiply") {
      cfg.render.premultiply = true;
    } else if (arg == "--matte") {
      cfg.render.matte = true;
    }

    // Behavior
    else if (arg == "-v" || arg == "--verbose") {
      cfg.verbose = true;
    }

    // Positional arguments
    else if (arg[0] != '-') {
      if (!cfg.art_path.has_value()) {
        cfg.art_path = arg;
      } else if (!cfg.palette_path.has_value()) {
        cfg.palette_path = arg;
      } else {
        return art2img::Error{art2img::error::invalid_argument, "Too many positional arguments"};
      }
    } else {
      return art2img::Error{art2img::error::invalid_argument, "Unknown option: " + arg};
    }
  }

  // Validate config based on mode
  if (cfg.run_mode == Mode::kList && !cfg.grp_path.has_value()) {
    return art2img::Error{art2img::error::invalid_argument, "--list requires --grp <file>"};
  }

  if (cfg.run_mode == Mode::kConvert) {
    // Need either GRP or ART file
    if (!cfg.grp_path.has_value() && !cfg.art_path.has_value()) {
      return art2img::Error{art2img::error::invalid_argument,
                            "No input file specified (use --grp or provide ART file)"};
    }

    // Standalone ART requires Palette
    if (!cfg.grp_path.has_value() && !cfg.palette_path.has_value()) {
      return art2img::Error{art2img::error::invalid_argument,
                            "Palette required for standalone ART file (use -p)"};
    }
  }

  return cfg;
}

}  // namespace art2img_cli
