// // This file implements the legacy API compatibility layer for art2img v2.0+
// // It forwards all legacy API calls to the new vNext modules

// #include <algorithm>
// #include <fstream>
// #include <sstream>

// #include <art2img/api.hpp>
// #include <art2img/legacy_api.hpp>

// using namespace art2img;
// using namespace art2img::legacy;
// using namespace art2img::types;

// // ============================================================================
// // LEGACY ART FILE IMPLEMENTATION
// // ============================================================================

// namespace art2img::legacy {

// ArtFile::ArtFile() : art_data_(std::make_unique<ArtData>()), is_open_(false) {}

// ArtFile::ArtFile(const std::filesystem::path& filename) : ArtFile() {
//   load(filename);
// }

// ArtFile::ArtFile(const uint8_t* data, size_t size) : ArtFile() {
//   load(data, size);
// }

// bool ArtFile::load(const std::filesystem::path& filename) {
//   filename_ = filename;

//   // Use new vNext API to load the ART file
//   auto result = load_art_bundle(filename);
//   if (!result) {
//     is_open_ = false;
//     return false;
//   }

//   art_data_ = std::make_unique<ArtData>(std::move(result.value()));
//   convert_art_data_to_legacy();
//   is_open_ = true;
//   return true;
// }

// bool ArtFile::load(const uint8_t* data, size_t size) {
//   // Use new vNext API to load from memory
//   auto span = std::span<const std::byte>(reinterpret_cast<const std::byte*>(data), size);
//   auto result = load_art_bundle(span);
//   if (!result) {
//     is_open_ = false;
//     return false;
//   }

//   art_data_ = std::make_unique<ArtData>(std::move(result.value()));
//   convert_art_data_to_legacy();
//   is_open_ = true;
//   return true;
// }

// void ArtFile::close() {
//   art_data_ = std::make_unique<ArtData>();
//   legacy_tiles_.clear();
//   is_open_ = false;
// }

// bool ArtFile::read_header() {
//   if (!art_data_ || !art_data_->is_valid()) {
//     return false;
//   }

//   // Header is already populated in convert_art_data_to_legacy()
//   return true;
// }

// bool ArtFile::read_tile_metadata() {
//   if (!art_data_ || !art_data_->is_valid()) {
//     return false;
//   }

//   // Tile metadata is already populated in convert_art_data_to_legacy()
//   return true;
// }

// bool ArtFile::read_tile_data(uint32_t index, std::vector<uint8_t>& buffer) {
//   if (!art_data_ || !art_data_->is_valid()) {
//     return false;
//   }

//   auto tile_view = make_tile_view(*art_data_, index);
//   if (!tile_view) {
//     return false;
//   }

//   buffer.resize(tile_view->pixels.size());
//   std::copy(tile_view->pixels.begin(), tile_view->pixels.end(), buffer.begin());
//   return true;
// }

// const ArtFile::Header& ArtFile::header() const {
//   return legacy_header_;
// }

// const std::vector<ArtFile::Tile>& ArtFile::tiles() const {
//   return legacy_tiles_;
// }

// bool ArtFile::is_open() const {
//   return is_open_;
// }

// const std::filesystem::path& ArtFile::filename() const {
//   return filename_;
// }

// const uint8_t* ArtFile::data() const {
//   if (!art_data_ || art_data_->pixels.empty()) {
//     return nullptr;
//   }
//   return art_data_->pixels.data();
// }

// size_t ArtFile::data_size() const {
//   if (!art_data_) {
//     return 0;
//   }
//   return art_data_->pixels.size();
// }

// bool ArtFile::has_data() const {
//   return art_data_ && !art_data_->pixels.empty();
// }

// void ArtFile::convert_art_data_to_legacy() {
//   if (!art_data_ || !art_data_->is_valid()) {
//     return;
//   }

//   // Convert header
//   legacy_header_.version = art_data_->version;
//   legacy_header_.start_tile = art_data_->tile_start;
//   legacy_header_.end_tile = art_data_->tile_end;
//   legacy_header_.num_tiles = static_cast<uint32_t>(art_data_->tile_count());

//   // Convert tiles
//   legacy_tiles_.clear();
//   legacy_tiles_.reserve(art_data_->tile_count());

//   for (size_t i = 0; i < art_data_->tile_count(); ++i) {
//     const auto& tile_view = art_data_->tiles[i];
//     legacy_tiles_.push_back(convert_tile_view_to_legacy(tile_view));
//   }
// }

// ArtFile::Tile ArtFile::convert_tile_view_to_legacy(const TileView& view) const {
//   Tile tile;
//   tile.width = view.width;
//   tile.height = view.height;
//   tile.offset = 0;  // Not used in legacy wrapper

//   // Convert animation data
//   tile.anim_data = view.animation.to_picanm();

//   return tile;
// }

// // ============================================================================
// // LEGACY PALETTE IMPLEMENTATION
// // ============================================================================

// Palette::Palette() : palette_(std::make_unique<::art2img::Palette>()), loaded_(false) {}

// bool Palette::load_from_file(const std::filesystem::path& filename) {
//   auto result = load_palette(filename);
//   if (!result) {
//     loaded_ = false;
//     return false;
//   }

//   palette_ = std::make_unique<::art2img::Palette>(std::move(result.value()));
//   update_raw_data();
//   loaded_ = true;
//   return true;
// }

// void Palette::load_build_engine_default() {
//   // For now, create a basic palette - this would need actual default palette
//   // data
//   palette_ = std::make_unique<::art2img::Palette>();
//   loaded_ = true;
//   update_raw_data();
// }

// void Palette::load_blood_default() {
//   // For now, create a basic palette - this would need actual Blood palette data
//   palette_ = std::make_unique<::art2img::Palette>();
//   loaded_ = true;
//   update_raw_data();
// }

// void Palette::load_duke3d_default() {
//   // For now, create a basic palette - this would need actual Duke3D palette
//   // data
//   palette_ = std::make_unique<::art2img::Palette>();
//   loaded_ = true;
//   update_raw_data();
// }

// bool Palette::load_from_memory(const uint8_t* data, size_t size) {
//   auto span = std::span<const std::byte>(reinterpret_cast<const std::byte*>(data), size);
//   auto result = load_palette(span);
//   if (!result) {
//     loaded_ = false;
//     return false;
//   }

//   palette_ = std::make_unique<::art2img::Palette>(std::move(result.value()));
//   update_raw_data();
//   loaded_ = true;
//   return true;
// }

// std::vector<uint8_t> Palette::get_bgr_data() const {
//   if (!palette_ || !loaded_) {
//     return {};
//   }

//   const auto& palette_data = palette_->palette_data();
//   return std::vector<uint8_t>(palette_data.begin(), palette_data.end());
// }

// const std::vector<uint8_t>& Palette::raw_data() const {
//   return raw_data_;
// }

// bool Palette::is_loaded() const {
//   return loaded_;
// }

// uint8_t Palette::get_red(size_t index) const {
//   if (!palette_ || index >= 256) {
//     return 0;
//   }

//   auto [r, g, b] = palette_entry_to_rgb(*palette_, static_cast<u8>(index));
//   return r;
// }

// uint8_t Palette::get_green(size_t index) const {
//   if (!palette_ || index >= 256) {
//     return 0;
//   }

//   auto [r, g, b] = palette_entry_to_rgb(*palette_, static_cast<u8>(index));
//   return g;
// }

// uint8_t Palette::get_blue(size_t index) const {
//   if (!palette_ || index >= 256) {
//     return 0;
//   }

//   auto [r, g, b] = palette_entry_to_rgb(*palette_, static_cast<u8>(index));
//   return b;
// }

// void Palette::update_raw_data() {
//   if (!palette_) {
//     raw_data_.clear();
//     return;
//   }

//   const auto& palette_data = palette_->palette_data();
//   raw_data_.assign(palette_data.begin(), palette_data.end());
// }

// // ============================================================================
// // LEGACY IMAGE WRITER IMPLEMENTATION
// // ============================================================================

// bool ImageWriter::write_image(const std::filesystem::path& filename,
//                               LegacyImageFormat format,
//                               const Palette& palette,
//                               const ArtFile::Tile& tile,
//                               const uint8_t* pixel_data,
//                               size_t pixel_data_size,
//                               const Options& options) {
//   // Convert legacy format to new format
//   ::art2img::ImageFormat new_format;
//   switch (format) {
//     case LegacyImageFormat::PNG:
//       new_format = ::art2img::ImageFormat::png;
//       break;
//     case LegacyImageFormat::TGA:
//       new_format = ::art2img::ImageFormat::tga;
//       break;
//     case LegacyImageFormat::BMP:
//       new_format = ::art2img::ImageFormat::bmp;
//       break;
//     default:
//       return false;
//   }

//   // Create a temporary tile view
//   TileView tile_view;
//   tile_view.width = tile.width;
//   tile_view.height = tile.height;
//   tile_view.pixels = std::span<const u8>(pixel_data, pixel_data_size);
//   tile_view.animation = TileAnimation(tile.anim_data);

//   // Convert to RGBA
//   auto conv_options = convert_legacy_options(options);
//   auto image_result = to_rgba(tile_view, *palette.palette_, conv_options);
//   if (!image_result) {
//     return false;
//   }

//   // Encode the image
//   auto encode_result = encode_image(image_view(image_result.value()), new_format);
//   if (!encode_result) {
//     return false;
//   }

//   // Write to file
//   auto byte_span =
//       std::span<const byte>(encode_result.value().data(), encode_result.value().size());
//   auto write_result = write_binary_file(filename, byte_span);
//   return write_result.has_value();
// }

// bool ImageWriter::write_image_to_memory(std::vector<uint8_t>& output,
//                                         LegacyImageFormat format,
//                                         const Palette& palette,
//                                         const ArtFile::Tile& tile,
//                                         const uint8_t* pixel_data,
//                                         size_t pixel_data_size,
//                                         const Options& options) {
//   // Convert legacy format to new format
//   ::art2img::ImageFormat new_format;
//   switch (format) {
//     case LegacyImageFormat::PNG:
//       new_format = ::art2img::ImageFormat::png;
//       break;
//     case LegacyImageFormat::TGA:
//       new_format = ::art2img::ImageFormat::tga;
//       break;
//     case LegacyImageFormat::BMP:
//       new_format = ::art2img::ImageFormat::bmp;
//       break;
//     default:
//       return false;
//   }

//   // Create a temporary tile view
//   TileView tile_view;
//   tile_view.width = tile.width;
//   tile_view.height = tile.height;
//   tile_view.pixels = std::span<const u8>(pixel_data, pixel_data_size);
//   tile_view.animation = TileAnimation(tile.anim_data);

//   // Convert to RGBA
//   auto conv_options = convert_legacy_options(options);
//   auto image_result = to_rgba(tile_view, *palette.palette_, conv_options);
//   if (!image_result) {
//     return false;
//   }

//   // Encode the image
//   auto encode_result = encode_image(image_view(image_result.value()), new_format);
//   if (!encode_result) {
//     return false;
//   }

//   // Copy to output vector
//   const auto& encoded_data = encode_result.value();
//   output.assign(encoded_data.begin(), encoded_data.end());
//   return true;
// }

// ConversionOptions ImageWriter::convert_legacy_options(const Options& options) {
//   ConversionOptions conv_options;
//   conv_options.apply_lookup = false;  // Not supported in legacy wrapper
//   conv_options.fix_transparency = options.fix_transparency;
//   conv_options.premultiply_alpha = options.premultiply_alpha;
//   conv_options.shade_index = 0;  // Not supported in legacy wrapper
//   return conv_options;
// }

// PngOptions ImageWriter::convert_png_options(const Options& options) {
//   PngOptions png_options;
//   png_options.compression_level = 6;  // Default
//   png_options.use_filters = true;
//   png_options.convert_to_grayscale = false;
//   return png_options;
// }

// TgaOptions ImageWriter::convert_tga_options(const Options& options) {
//   TgaOptions tga_options;
//   tga_options.use_rle = true;
//   tga_options.include_alpha = options.enable_alpha;
//   tga_options.flip_vertically = false;
//   return tga_options;
// }

// BmpOptions ImageWriter::convert_bmp_options(const Options& options) {
//   BmpOptions bmp_options;
//   bmp_options.include_alpha = options.enable_alpha;
//   bmp_options.flip_vertically = false;
//   return bmp_options;
// }

// // ============================================================================
// // LEGACY IMAGE VIEW IMPLEMENTATION
// // ============================================================================

// const uint8_t* ImageView::pixel_data() const {
//   if (!parent || tile_index >= parent->tiles.size()) {
//     return nullptr;
//   }

//   const auto& tile = parent->tiles[tile_index];
//   if (tile.is_empty()) {
//     return nullptr;
//   }

//   return parent->art_data + tile.offset;
// }

// uint16_t ImageView::width() const {
//   if (!parent || tile_index >= parent->tiles.size()) {
//     return 0;
//   }
//   return parent->tiles[tile_index].width;
// }

// uint16_t ImageView::height() const {
//   if (!parent || tile_index >= parent->tiles.size()) {
//     return 0;
//   }
//   return parent->tiles[tile_index].height;
// }

// size_t ImageView::size() const {
//   return static_cast<size_t>(width()) * static_cast<size_t>(height());
// }

// uint32_t ImageView::anim_frames() const {
//   return require_tile().anim_frames();
// }

// uint32_t ImageView::anim_type() const {
//   return require_tile().anim_type();
// }

// int8_t ImageView::x_offset() const {
//   return require_tile().x_offset();
// }

// int8_t ImageView::y_offset() const {
//   return require_tile().y_offset();
// }

// uint32_t ImageView::anim_speed() const {
//   return require_tile().anim_speed();
// }

// uint32_t ImageView::other_flags() const {
//   return require_tile().other_flags();
// }

// bool ImageView::save_to_image(const std::filesystem::path& path,
//                               LegacyImageFormat format,
//                               ImageWriter::Options options) const {
//   if (!parent || !parent->palette) {
//     return false;
//   }

//   const auto& tile = require_tile();
//   const auto* pixel_data = this->pixel_data();

//   return ImageWriter::write_image(path,
//                                   format,
//                                   *parent->palette,
//                                   tile,
//                                   pixel_data,
//                                   size(),
//                                   options);
// }

// bool ImageView::save_to_png(const std::filesystem::path& path, ImageWriter::Options options) const {
//   return save_to_image(path, LegacyImageFormat::PNG, options);
// }

// bool ImageView::save_to_tga(const std::filesystem::path& path) const {
//   return save_to_image(path, LegacyImageFormat::TGA, ImageWriter::Options());
// }

// bool ImageView::save_to_bmp(const std::filesystem::path& path) const {
//   return save_to_image(path, LegacyImageFormat::BMP, ImageWriter::Options());
// }

// std::vector<uint8_t> ImageView::extract_to_image(LegacyImageFormat format,
//                                                  ImageWriter::Options options) const {
//   if (!parent || !parent->palette) {
//     return {};
//   }

//   const auto& tile = require_tile();
//   const auto* pixel_data = this->pixel_data();

//   std::vector<uint8_t> output;
//   if (ImageWriter::write_image_to_memory(output,
//                                          format,
//                                          *parent->palette,
//                                          tile,
//                                          pixel_data,
//                                          size(),
//                                          options)) {
//     return output;
//   }
//   return {};
// }

// std::vector<uint8_t> ImageView::extract_to_png(ImageWriter::Options options) const {
//   return extract_to_image(LegacyImageFormat::PNG, options);
// }

// std::vector<uint8_t> ImageView::extract_to_tga() const {
//   return extract_to_image(LegacyImageFormat::TGA, ImageWriter::Options());
// }

// std::vector<uint8_t> ImageView::extract_to_bmp() const {
//   return extract_to_image(LegacyImageFormat::BMP, ImageWriter::Options());
// }

// const ArtFile::Tile& ImageView::require_tile() const {
//   if (!parent) {
//     throw ArtException("Invalid ImageView state");
//   }
//   return parent->get_tile(tile_index);
// }

// // ============================================================================
// // LEGACY EXTRACTOR API IMPLEMENTATION
// // ============================================================================

// ExtractorAPI::ExtractorAPI()
//     : art_file_(std::make_unique<ArtFile>()), palette_(std::make_unique<Palette>()) {}

// bool ExtractorAPI::load_art_file(const std::filesystem::path& filename) {
//   return art_file_->load(filename);
// }

// bool ExtractorAPI::load_palette_file(const std::filesystem::path& filename) {
//   return palette_->load_from_file(filename);
// }

// bool ExtractorAPI::load_art_from_memory(const uint8_t* data, size_t size) {
//   return art_file_->load(data, size);
// }

// bool ExtractorAPI::load_palette_from_memory(const uint8_t* data, size_t size) {
//   return palette_->load_from_memory(data, size);
// }

// void ExtractorAPI::set_duke3d_default_palette() {
//   palette_->load_duke3d_default();
// }

// void ExtractorAPI::set_blood_default_palette() {
//   palette_->load_blood_default();
// }

// ExtractionResult ExtractorAPI::extract_tile(uint32_t tile_index,
//                                             LegacyImageFormat format,
//                                             ImageWriter::Options options) {
//   if (!art_file_->is_open() || !palette_->is_loaded()) {
//     ExtractionResult result;
//     result.success = false;
//     result.error_message = "ART file or palette not loaded";
//     return result;
//   }

//   if (tile_index >= art_file_->tiles().size()) {
//     ExtractionResult result;
//     result.success = false;
//     result.error_message = "Tile index out of range";
//     return result;
//   }

//   const auto& tile = art_file_->tiles()[tile_index];
//   if (tile.is_empty()) {
//     ExtractionResult result;
//     result.success = false;
//     result.error_message = "Tile is empty";
//     return result;
//   }

//   // Get tile data
//   std::vector<uint8_t> tile_data;
//   if (!art_file_->read_tile_data(tile_index, tile_data)) {
//     ExtractionResult result;
//     result.success = false;
//     result.error_message = "Failed to read tile data";
//     return result;
//   }

//   // Create tile view for conversion
//   TileView tile_view;
//   tile_view.width = tile.width;
//   tile_view.height = tile.height;
//   tile_view.pixels = std::span<const u8>(tile_data.data(), tile_data.size());
//   tile_view.animation = TileAnimation(tile.anim_data);

//   // Convert to new format
//   ::art2img::ImageFormat new_format;
//   std::string format_str;
//   switch (format) {
//     case LegacyImageFormat::PNG:
//       new_format = ::art2img::ImageFormat::png;
//       format_str = "png";
//       break;
//     case LegacyImageFormat::TGA:
//       new_format = ::art2img::ImageFormat::tga;
//       format_str = "tga";
//       break;
//     case LegacyImageFormat::BMP:
//       new_format = ::art2img::ImageFormat::bmp;
//       format_str = "bmp";
//       break;
//     default:
//       ExtractionResult result;
//       result.success = false;
//       result.error_message = "Unsupported image format";
//       return result;
//   }

//   auto conv_options = ImageWriter::convert_legacy_options(options);
//   auto image_result = to_rgba(tile_view, *palette_->palette_, conv_options);
//   if (!image_result) {
//     ExtractionResult result;
//     result.success = false;
//     result.error_message = image_result.error().message;
//     return result;
//   }

//   auto encode_result = encode_image(image_view(image_result.value()), new_format);
//   return create_extraction_result(encode_result, tile_index, format_str, tile_view);
// }

// ExtractionResult ExtractorAPI::extract_tile_png(uint32_t tile_index, ImageWriter::Options options) {
//   return extract_tile(tile_index, LegacyImageFormat::PNG, options);
// }

// ExtractionResult ExtractorAPI::extract_tile_tga(uint32_t tile_index, ImageWriter::Options options) {
//   return extract_tile(tile_index, LegacyImageFormat::TGA, options);
// }

// ExtractionResult ExtractorAPI::extract_tile_bmp(uint32_t tile_index, ImageWriter::Options options) {
//   return extract_tile(tile_index, LegacyImageFormat::BMP, options);
// }

// std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles(LegacyImageFormat format,
//                                                               ImageWriter::Options options) {
//   std::vector<ExtractionResult> results;

//   if (!art_file_->is_open() || !palette_->is_loaded()) {
//     ExtractionResult result;
//     result.success = false;
//     result.error_message = "ART file or palette not loaded";
//     results.push_back(result);
//     return results;
//   }

//   uint32_t tile_count = get_tile_count();
//   results.reserve(tile_count);

//   for (uint32_t i = 0; i < tile_count; ++i) {
//     results.push_back(extract_tile(i, format, options));
//   }

//   return results;
// }

// std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles_png(ImageWriter::Options options) {
//   return extract_all_tiles(LegacyImageFormat::PNG, options);
// }

// std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles_tga(ImageWriter::Options options) {
//   return extract_all_tiles(LegacyImageFormat::TGA, options);
// }

// std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles_bmp(ImageWriter::Options options) {
//   return extract_all_tiles(LegacyImageFormat::BMP, options);
// }

// bool ExtractorAPI::is_art_loaded() const {
//   return art_file_ && art_file_->is_open();
// }

// bool ExtractorAPI::is_palette_loaded() const {
//   return palette_ && palette_->is_loaded();
// }

// uint32_t ExtractorAPI::get_tile_count() const {
//   return art_file_ ? static_cast<uint32_t>(art_file_->tiles().size()) : 0;
// }

// ArtView ExtractorAPI::get_art_view() const {
//   ArtView view;

//   if (!art_file_->is_open() || !palette_->is_loaded()) {
//     return view;
//   }

//   view.art_data = art_file_->data();
//   view.art_size = art_file_->data_size();
//   view.palette = palette_.get();
//   view.header = art_file_->header();
//   view.tiles = art_file_->tiles();

//   return view;
// }

// bool ExtractorAPI::write_animation_data(const std::string& art_file_path,
//                                         const std::string& output_dir) const {
//   // This is a simplified implementation - the full implementation would
//   // need to generate animation manifest files
//   std::filesystem::path output_path(output_dir);
//   auto ensure_dir = ensure_directory_exists(output_path);
//   if (!ensure_dir) {
//     return false;
//   }

//   std::string ini_content = generate_animation_ini_content(art_file_path);
//   std::filesystem::path ini_path = output_path / "animation.ini";

//   auto write_result = write_text_file(ini_path, ini_content);
//   return write_result.has_value();
// }

// std::string ExtractorAPI::generate_animation_ini_content(const std::string& art_file_path) const {
//   std::ostringstream ini;
//   ini << "; Animation data for " << art_file_path << "\n";
//   ini << "[Animation]\n";

//   if (!art_file_->is_open()) {
//     return ini.str();
//   }

//   uint32_t tile_count = get_tile_count();
//   for (uint32_t i = 0; i < tile_count; ++i) {
//     const auto& tile = art_file_->tiles()[i];
//     if (tile.anim_frames() > 1) {
//       ini << "tile" << i << "_frames=" << tile.anim_frames() << "\n";
//       ini << "tile" << i << "_type=" << get_animation_type_string(tile.anim_type()) << "\n";
//       ini << "tile" << i << "_speed=" << tile.anim_speed() << "\n";
//       ini << "tile" << i << "_xoffset=" << static_cast<int>(tile.x_offset()) << "\n";
//       ini << "tile" << i << "_yoffset=" << static_cast<int>(tile.y_offset()) << "\n";
//       ini << "\n";
//     }
//   }

//   return ini.str();
// }

// std::string ExtractorAPI::get_animation_type_string(uint32_t anim_type) const {
//   switch (anim_type) {
//     case 0:
//       return "none";
//     case 1:
//       return "oscillating";
//     case 2:
//       return "forward";
//     case 3:
//       return "backward";
//     default:
//       return "unknown";
//   }
// }

// ExtractionResult ExtractorAPI::create_extraction_result(
//     const std::expected<std::vector<byte>, Error>& result,
//     uint32_t tile_index,
//     const std::string& format,
//     const TileView& tile_view) const {
//   ExtractionResult extraction_result;

//   if (!result) {
//     extraction_result.success = false;
//     extraction_result.error_message = result.error().message;
//     return extraction_result;
//   }

//   extraction_result.success = true;
//   extraction_result.format = format;
//   extraction_result.tile_index = tile_index;
//   extraction_result.width = tile_view.width;
//   extraction_result.height = tile_view.height;

//   // Copy encoded data
//   const auto& encoded_data = result.value();
//   extraction_result.image_data.resize(encoded_data.size());
//   std::transform(encoded_data.begin(),
//                  encoded_data.end(),
//                  extraction_result.image_data.begin(),
//                  [](byte b) { return static_cast<uint8_t>(b); });

//   // Copy animation data
//   extraction_result.anim_frames = tile_view.animation.frame_count;
//   extraction_result.anim_type = static_cast<uint32_t>(tile_view.animation.type);
//   extraction_result.x_offset = tile_view.animation.x_center_offset;
//   extraction_result.y_offset = tile_view.animation.y_center_offset;
//   extraction_result.anim_speed = tile_view.animation.speed;
//   extraction_result.other_flags = 0;  // Not used in new implementation

//   return extraction_result;
// }

// }  // namespace art2img::legacy