#include "cli.hpp"
#include <iostream>
#include <getopt.h>
#include <filesystem>

namespace art2image {

const char* const CLI::HELP_TEXT = 
    "Usage: art2image [OPTIONS] <ART_FILE|ART_DIRECTORY>\n\n"
    "Extract pictures from ART files to TGA or PNG format\n\n"
    "Options:\n"
    "  -o, --output DIR     Output directory (default: current)\n"
    "  -t, --threads N      Number of threads (default: CPU cores)\n"
    "  -p, --palette FILE   Palette file path (default: auto-detect)\n"
     "  -f, --format FMT     Output format: tga or png (default: png)\n"
     "  -F, --fix-transparency  Enable magenta transparency fix (default)\n"
     "  -N, --no-fix-transparency  Disable magenta transparency fix\n"
     "  -q, --quiet          Suppress verbose output\n"
     "  -n, --no-anim        Don't generate animdata.ini\n"
     "  -m, --merge-anim     Merge animation data into single file (directory mode)\n"
     "  -h, --help           Show this help message\n"
     "  -v, --version        Show version information\n\n"
     "Examples:\n"
     "  art2image -o ./output -t 4 tiles000.art\n"
     "  art2image -p palette.dat tiles000.art\n"
     "  art2image -f tga tiles000.art        # TGA format\n"
     "  art2image assets/                    # Process all ART files in assets/\n";

const char* const CLI::VERSION_TEXT = 
    "art2image 1.0\n"
    "Multi-threaded ART to image converter (TGA/PNG)\n"
    "Based on original art2tga by Mathieu Olivier\n"
    "Coded by Raúl Correia\n";

CLI::Options CLI::parse_arguments(int argc, char* argv[]) {
    Options options;
    
    static struct option long_options[] = {
         {"output", required_argument, 0, 'o'},
        {"threads", required_argument, 0, 't'},
        {"palette", required_argument, 0, 'p'},
        {"format", required_argument, 0, 'f'},
        {"fix-transparency", no_argument, 0, 'F'},
        {"no-fix-transparency", no_argument, 0, 'N'},
        {"tolerance", required_argument, 0, 'c'},
        {"quiet", no_argument, 0, 'q'},
        {"no-anim", no_argument, 0, 'n'},
        {"merge-anim", no_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "o:t:p:f:FNc:qnmhv", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'o':
                options.extractor_options.output_dir = optarg;
                break;
            case 't':
                try {
                    int threads = std::stoi(optarg);
                    options.extractor_options.num_threads = std::max(1, threads);
                } catch (...) {
                    std::cerr << "Warning: Invalid thread count, using default" << std::endl;
                }
                break;
            case 'p':
                options.palette_file = optarg;
                break;
            case 'f':
                if (std::string(optarg) == "png") {
                    options.extractor_options.format = ArtExtractor::OutputFormat::PNG;
                } else if (std::string(optarg) == "tga") {
                    options.extractor_options.format = ArtExtractor::OutputFormat::TGA;
                } else {
                    std::cerr << "Error: Invalid format. Use 'tga' or 'png'\n\n";
                    print_usage(argv[0]);
                    exit(1);
                }
                 break;
            case 'F':
                // Enable magenta transparency fix (default behavior)
                options.extractor_options.png_options.enable_magenta_transparency = true;
                break;
            case 'N':
                // Disable magenta transparency fix
                options.extractor_options.png_options.enable_magenta_transparency = false;
                break;
            case 'm':
                options.extractor_options.merge_animation_data = true;
                break;
            case 'c':
                std::cerr << "Warning: Color tolerance option (-c) is no longer supported" << std::endl;
                break;
            case 'q':
                options.extractor_options.verbose = false;
                break;
            case 'n':
                options.extractor_options.dump_animation = false;
                break;
            case 'h':
                options.show_help = true;
                break;
            case 'v':
                print_version();
                exit(0);
                break;
            default:
                print_usage(argv[0]);
                exit(1);
        }
    }
    
    // Get input argument (file or directory)
    if (optind < argc) {
        std::string input_path = argv[optind];
        std::filesystem::path path(input_path);
        
        if (std::filesystem::is_directory(path)) {
            // Directory input - process all ART files in directory
            options.art_directory = input_path;
            options.process_directory = true;
        } else {
            // Single file input
            options.art_file = input_path;
            
            // Validate file extension
            if (path.extension() != ".art" && path.extension() != ".ART") {
                std::cerr << "Error: Input file must have .art extension: " << options.art_file << "\n\n";
                print_usage(argv[0]);
                exit(1);
            }
        }
    } else {
        // No input specified - show help
        print_usage(argv[0]);
        exit(1);
    }
    
    if (options.show_help) {
        print_usage(argv[0]);
        exit(0);
    }
    
    if (!options.is_valid()) {
        if (options.art_file.empty() && options.art_directory.empty()) {
            std::cerr << "Error: No ART file or directory specified\n\n";
        }
        print_usage(argv[0]);
        exit(1);
    }
    
    return options;
}

void CLI::print_usage(const char* /*program_name*/) {
    std::cout << HELP_TEXT;
}

void CLI::print_version() {
    std::cout << VERSION_TEXT;
}

} // namespace art2image