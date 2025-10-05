#pragma once

#include <CLI/CLI.hpp>

#include <memory>
#include <string>

#include "config.hpp"

namespace art2img {

class CliAppBuilder {
 public:
  CliAppBuilder();

  CliAppBuilder& with_banner(std::string banner);
  CliAppBuilder& with_footer(std::string footer);

  [[nodiscard]] std::unique_ptr<CLI::App> build(CliOptions& options) const;

  [[nodiscard]] const std::string& banner() const noexcept { return banner_; }
  [[nodiscard]] const std::string& footer() const noexcept { return footer_; }

  static std::string default_banner();
  static std::string default_footer();

 private:
  std::string banner_;
  std::string footer_;
};

}  // namespace art2img
