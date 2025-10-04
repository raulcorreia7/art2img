#pragma once

#include <string>

struct CliOptions {
  std::string input_path;
  std::string output_dir = ".";
  std::string palette_file;
  std::string format = "png";
  bool fix_transparency = true;
  bool quiet = false;
  bool no_anim = false;
  bool merge_anim = false;
};

struct ProcessingOptions {
  std::string palette_file;
  std::string output_dir;
  std::string format = "png";
  bool fix_transparency = true;
  bool verbose = false;
  bool dump_animation = true;
  bool merge_animation_data = false;
};