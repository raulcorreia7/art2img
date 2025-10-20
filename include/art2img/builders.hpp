#pragma once

#include <art2img/art.hpp>
#include <art2img/convert.hpp>
#include <art2img/export.hpp>

namespace art2img {

/// @brief Builder for ConversionOptions
class ConversionOptionsBuilder {
 private:
  ConversionOptions options_;

 public:
  /// @brief Default constructor
  ConversionOptionsBuilder() = default;

  /// @brief Set whether to apply palette remapping
  /// @param apply Whether to apply remapping
  /// @return Reference to this builder for chaining
  ConversionOptionsBuilder& apply_lookup(bool apply) noexcept {
    options_.apply_lookup = apply;
    return *this;
  }

  /// @brief Set whether to fix transparency
  /// @param fix Whether to fix transparency
  /// @return Reference to this builder for chaining
  ConversionOptionsBuilder& fix_transparency(bool fix) noexcept {
    options_.fix_transparency = fix;
    return *this;
  }

  /// @brief Set whether to premultiply alpha
  /// @param premultiply Whether to premultiply alpha
  /// @return Reference to this builder for chaining
  ConversionOptionsBuilder& premultiply_alpha(bool premultiply) noexcept {
    options_.premultiply_alpha = premultiply;
    return *this;
  }

  /// @brief Set whether to apply matte hygiene
  /// @param apply Whether to apply matte hygiene
  /// @return Reference to this builder for chaining
  ConversionOptionsBuilder& matte_hygiene(bool apply) noexcept {
    options_.matte_hygiene = apply;
    return *this;
  }

  /// @brief Set shade table index
  /// @param index Shade table index (0 = no shading)
  /// @return Reference to this builder for chaining
  ConversionOptionsBuilder& shade_index(types::u8 index) noexcept {
    options_.shade_index = index;
    return *this;
  }

  /// @brief Build the ConversionOptions object
  /// @return The configured ConversionOptions
  ConversionOptions build() const noexcept { return options_; }
};

/// @brief Builder for ExportOptions
class ExportOptionsBuilder {
 private:
  ExportOptions options_;

 public:
  /// @brief Default constructor
  ExportOptionsBuilder() = default;

  /// @brief Set output directory
  /// @param dir Output directory path
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& output_dir(const std::filesystem::path& dir) {
    options_.output_dir = dir;
    return *this;
  }

  /// @brief Set image format
  /// @param format Image format for export
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& format(ImageFormat format) noexcept {
    options_.format = format;
    return *this;
  }

  /// @brief Set whether to organize by format
  /// @param organize Whether to create subdirectories per format
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& organize_by_format(bool organize) noexcept {
    options_.organize_by_format = organize;
    return *this;
  }

  /// @brief Set whether to organize by ART file
  /// @param organize Whether to create subdirectories per ART file
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& organize_by_art_file(bool organize) noexcept {
    options_.organize_by_art_file = organize;
    return *this;
  }

  /// @brief Set filename prefix
  /// @param prefix Prefix for generated filenames
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& filename_prefix(const std::string& prefix) {
    options_.filename_prefix = prefix;
    return *this;
  }

  /// @brief Set conversion options
  /// @param options Conversion options
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& conversion_options(
      const ConversionOptions& options) noexcept {
    options_.conversion_options = options;
    return *this;
  }

  /// @brief Set conversion options using a builder
  /// @param builder Conversion options builder
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& conversion_options(
      const ConversionOptionsBuilder& builder) noexcept {
    options_.conversion_options = builder.build();
    return *this;
  }

  /// @brief Set whether to enable parallel processing
  /// @param enable Whether to enable parallel processing
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& enable_parallel(bool enable) noexcept {
    options_.enable_parallel = enable;
    return *this;
  }

  /// @brief Set maximum number of threads
  /// @param threads Maximum threads (0 = auto-detect)
  /// @return Reference to this builder for chaining
  ExportOptionsBuilder& max_threads(std::size_t threads) noexcept {
    options_.max_threads = threads;
    return *this;
  }

  /// @brief Build the ExportOptions object
  /// @return The configured ExportOptions
  ExportOptions build() const { return options_; }
};

/// @brief Builder for AnimationExportConfig
class AnimationExportConfigBuilder {
 private:
  AnimationExportConfig config_;

 public:
  /// @brief Default constructor
  AnimationExportConfigBuilder() = default;

  /// @brief Set output directory
  /// @param dir Output directory path
  /// @return Reference to this builder for chaining
  AnimationExportConfigBuilder& output_dir(const std::filesystem::path& dir) {
    config_.output_dir = dir;
    return *this;
  }

  /// @brief Set base filename
  /// @param name Base filename for animation tiles
  /// @return Reference to this builder for chaining
  AnimationExportConfigBuilder& base_name(const std::string& name) {
    config_.base_name = name;
    return *this;
  }

  /// @brief Set whether to include non-animated tiles
  /// @param include Whether to include tiles without animation data
  /// @return Reference to this builder for chaining
  AnimationExportConfigBuilder& include_non_animated(bool include) noexcept {
    config_.include_non_animated = include;
    return *this;
  }

  /// @brief Set whether to generate INI file
  /// @param generate Whether to generate INI file with animation metadata
  /// @return Reference to this builder for chaining
  AnimationExportConfigBuilder& generate_ini(bool generate) noexcept {
    config_.generate_ini = generate;
    return *this;
  }

  /// @brief Set INI filename
  /// @param filename INI filename
  /// @return Reference to this builder for chaining
  AnimationExportConfigBuilder& ini_filename(const std::string& filename) {
    config_.ini_filename = filename;
    return *this;
  }

  /// @brief Set image format
  /// @param format Image format for exported tiles
  /// @return Reference to this builder for chaining
  AnimationExportConfigBuilder& image_format(ImageFormat format) noexcept {
    config_.image_format = format;
    return *this;
  }

  /// @brief Set whether to include image file references
  /// @param include Whether to include image file references in INI output
  /// @return Reference to this builder for chaining
  AnimationExportConfigBuilder& include_image_references(
      bool include) noexcept {
    config_.include_image_references = include;
    return *this;
  }

  /// @brief Build the AnimationExportConfig object
  /// @return The configured AnimationExportConfig
  AnimationExportConfig build() const { return config_; }
};

}  // namespace art2img