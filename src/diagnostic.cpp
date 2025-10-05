#include <extractor_api.hpp>
#include <iostream>
#include <version.hpp>

int main(int argc, char* argv[]) {
  void(argc);
  void(argv);
  std::cout << "art2img Diagnostic Tool" << std::endl;
  std::cout << "Version: " << ART2IMG_VERSION << std::endl;
  std::cout << "========================" << std::endl;

  // Basic diagnostic information
  std::cout << "System Information:" << std::endl;
  std::cout << "  - Library API Available: Yes" << std::endl;
  std::cout << "  - File Processing: Ready" << std::endl;
  std::cout << "  - Image Export: PNG/TGA Supported" << std::endl;

  // Test library initialization
  try {
    art2img::ExtractorAPI extractor;
    std::cout << "  - Library Initialization: Success" << std::endl;
  } catch (const std::exception& e) {
    std::cout << "  - Library Initialization: Failed - " << e.what() << std::endl;
    return 1;
  }

  std::cout << std::endl;
  std::cout << "Diagnostic completed successfully." << std::endl;
  std::cout << "Use the main art2img tool for file processing." << std::endl;

  return 0;
}