#include "ops.hpp"

#include <atomic>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

namespace art2img_cli {

std::optional<std::string> auto_detect_art(const art2img::GrpFile& grp) noexcept {
    for (const auto& name : grp.list()) {
        if (name.size() >= 4 && name.substr(name.size() - 4) == ".art") {
            if (name.find("tiles") == 0 || name.find("TILES") == 0) {
                return name;
            }
        }
    }
    return std::nullopt;
}

art2img::ConvertOptions make_convert_opts(const Config& cfg) noexcept {
    art2img::ConvertOptions opts;
    opts.render = cfg.render;
    opts.format = cfg.fmt;
    opts.verbose = cfg.verbose;
    opts.output_prefix = cfg.output_dir;
    opts.output_prefix += '/';

    switch (cfg.fmt) {
    case art2img::Format::png:
        opts.output_suffix = ".png";
        break;
    case art2img::Format::tga:
        opts.output_suffix = ".tga";
        break;
    case art2img::Format::bmp:
        opts.output_suffix = ".bmp";
        break;
    }

    return opts;
}

int cmd_list(const Config& cfg) noexcept {
    // Load GRP
    auto grp = art2img::GrpFile::load(cfg.grp_path.value());
    if (!grp) {
        std::cerr << "Error loading GRP: " << grp.message() << "\n";
        return 1;
    }

    std::cout << "GRP file: " << cfg.grp_path.value() << "\n";
    std::cout << "Files: " << grp.value().count() << "\n\n";

    // Categorize files
    std::vector<std::string> arts, cons, other;
    for (const auto& name : grp.value().list()) {
        if (name.size() >= 4 && name.substr(name.size() - 4) == ".art") {
            arts.push_back(name);
        } else if (name.size() >= 4 && name.substr(name.size() - 4) == ".con") {
            cons.push_back(name);
        } else {
            other.push_back(name);
        }
    }

    // Print categorized
    if (!arts.empty()) {
        std::cout << "ART files (tile data):\n";
        for (const auto& n : arts) {
            std::cout << "  " << n << "\n";
        }
        std::cout << "\n";
    }

    if (!cons.empty()) {
        std::cout << "CON files (configuration):\n";
        for (const auto& n : cons) {
            std::cout << "  " << n << "\n";
        }
        std::cout << "\n";
    }

    if (!other.empty()) {
        std::cout << "Other files (" << other.size() << " files):\n";
        for (const auto& n : other) {
            std::cout << "  " << n << "\n";
        }
    }

    return 0;
}

int cmd_convert(const Config& cfg) noexcept {
    using namespace art2img;

    ArtFile art;
    Palette pal;

    // Load from GRP or standalone files
    if (cfg.grp_path.has_value()) {
        if (cfg.verbose) {
            std::clog << "Loading GRP: " << cfg.grp_path.value() << "\n";
        }

        auto grp = GrpFile::load(cfg.grp_path.value());
        if (!grp) {
            std::cerr << "Error loading GRP: " << grp.message() << "\n";
            return 1;
        }

        // Load Palette
        if (cfg.palette_path.has_value()) {
            if (cfg.verbose) {
                std::clog << "Loading Palette: " << cfg.palette_path.value() << "\n";
            }
            auto p = Palette::load(cfg.palette_path.value());
            if (!p) {
                std::cerr << "Error loading Palette: " << p.message() << "\n";
                return 1;
            }
            pal = p.value();
        } else {
            if (cfg.verbose) {
                std::clog << "Loading Palette from GRP\n";
            }
            auto p = Palette::from_grp(grp.value());
            if (!p) {
                std::cerr << "Error loading Palette: " << p.message() << "\n";
                return 1;
            }
            pal = p.value();
        }

        // Find/load ART
        std::string art_name;
        if (cfg.art_name.has_value()) {
            art_name = cfg.art_name.value();
        } else {
            auto detected = auto_detect_art(grp.value());
            if (!detected) {
                std::cerr << "Error: No ART file found in GRP (use --art to specify)\n";
                return 1;
            }
            art_name = detected.value();
        }

        if (cfg.verbose) {
            std::clog << "Loading ART: " << art_name << "\n";
        }
        auto a = ArtFile::from_grp(grp.value(), art_name);
        if (!a) {
            std::cerr << "Error loading ART: " << a.message() << "\n";
            return 1;
        }
        art = std::move(a.value());
    } else {
        // Load standalone ART
        if (cfg.verbose) {
            std::clog << "Loading ART: " << cfg.art_path.value() << "\n";
        }
        auto a = ArtFile::load(cfg.art_path.value());
        if (!a) {
            std::cerr << "Error loading ART: " << a.message() << "\n";
            return 1;
        }
        art = std::move(a.value());

        // Load Palette
        if (cfg.verbose) {
            std::clog << "Loading Palette: " << cfg.palette_path.value() << "\n";
        }
        auto p = Palette::load(cfg.palette_path.value());
        if (!p) {
            std::cerr << "Error loading Palette: " << p.message() << "\n";
            return 1;
        }
        pal = p.value();
    }

    // Print info
    if (cfg.verbose) {
        std::clog << "ART: " << art.tile_count() << " tiles\n";
        std::clog << "Palette: " << pal.shade_count() << " shade tables\n";
        std::clog << "Output: " << cfg.output_dir << "/\n";
        std::clog << "Format: "
                  << (cfg.fmt == Format::png   ? "PNG"
                      : cfg.fmt == Format::tga ? "TGA"
                                               : "BMP")
                  << "\n";
    }

    // Create output directory if needed
    std::error_code ec;
    std::filesystem::create_directories(cfg.output_dir, ec);

    // Determine thread count
    size_t num_jobs = cfg.jobs > 0 ? cfg.jobs : std::thread::hardware_concurrency();
    if (num_jobs == 0)
        num_jobs = 1;
    if (!cfg.parallel)
        num_jobs = 1;

    if (cfg.verbose) {
        std::clog << "Threads: " << num_jobs << "\n";
    }

    auto copts = make_convert_opts(cfg);

    size_t count = art.tile_count();
    std::atomic<size_t> success{0};
    std::atomic<bool> has_error{false};
    std::string error_msg;

    auto worker = [&](size_t start, size_t end) {
        for (size_t i = start; i < end && !has_error; ++i) {
            auto size = art.get_tile_size(i);
            if (!size || size->width == 0 || size->height == 0) {
                continue;
            }

            auto img_result = art2img::extract_tile(art, pal, i, cfg.render);
            if (!img_result.ok()) {
                error_msg = "Tile " + std::to_string(i) + ": " + img_result.message();
                has_error = true;
                return;
            }

            auto enc_result = art2img::encode(img_result.value(), cfg.fmt);
            if (!enc_result.ok()) {
                error_msg = "Tile " + std::to_string(i) + ": " + enc_result.message();
                has_error = true;
                return;
            }

            std::string filename = copts.output_prefix + std::to_string(i) + copts.output_suffix;
            auto write_result = art2img::write_file(filename, enc_result.value());
            if (!write_result.ok()) {
                error_msg = "Tile " + std::to_string(i) + ": " + write_result.message();
                has_error = true;
                return;
            }

            ++success;
        }
    };

    // Single-threaded
    if (num_jobs == 1) {
        worker(0, count);
    } else {
        // Multi-threaded
        std::vector<std::thread> threads;
        size_t chunk_size = (count + num_jobs - 1) / num_jobs;

        for (size_t t = 0; t < num_jobs; ++t) {
            size_t start = t * chunk_size;
            size_t end = std::min(start + chunk_size, count);
            threads.emplace_back(worker, start, end);
        }

        for (auto& th : threads) {
            th.join();
        }
    }

    if (has_error) {
        std::cerr << "Error: " << error_msg << "\n";
        return 1;
    }

    if (!cfg.quiet) {
        std::cout << "Converted " << success.load() << " tiles\n";
    }

    return 0;
}

}  // namespace art2img_cli
