#pragma once

#include "extractor.hpp"
#include <string>

namespace art2img {

class CLI {
public:
    struct Options {
        std::string art_file;           // Single ART file path
        std::string art_directory;      // Directory containing ART files
        std::string palette_file;       // User-specified palette file path
        ArtExtractor::Options extractor_options;
        bool show_help = false;
        bool process_directory = false; // Flag to indicate directory processing
        
        bool is_valid() const {
            return (!art_file.empty() || !art_directory.empty()) && extractor_options.is_valid();
        }
    };
    
    static Options parse_arguments(int argc, char* argv[]);
    static void print_usage(const char* program_name);
    static void print_version();
    
private:
    static const char* const HELP_TEXT;
    static const char* const VERSION_TEXT;
};

} // namespace art2img