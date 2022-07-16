// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#include <iostream>

#include "cxxopts/cxxopts.hpp"
#include "pe.h"
#include "version.h"
#include "walker.h"

int main(int argc, char **argv) {
  try {
    windep::exc::SeException::SetTranslator();
    cxxopts::Options options("windep.exe",
                             "Windows PE format dependency resolver");
    options.positional_help("<root image>");
    options.parse_positional({"image"});
    options.add_options()("i,image", "Root image",
                          cxxopts::value<std::string>())(
        "f,functions", "Enable functions output",
        cxxopts::value<bool>()->default_value("false"))(
        "d,delayed", "Enable resolving of delayed imports",
        cxxopts::value<bool>()->default_value("false"))(
        "h,help", "Print help", cxxopts::value<bool>()->default_value("false"))(
        "v,version", "Print version",
        cxxopts::value<bool>()->default_value("false"));
    auto args = options.parse(argc, argv);

    if (args.count("help")) {
      std::cout << options.help() << std::endl;
      return 0;
    } else if (args.count("version")) {
      std::cout << VERSION << std::endl;
      return 0;
    }

    const auto &image = args["image"].as<std::string>();
    const auto is_delayed = args["delayed"].as<bool>();
    const auto pe_image_factory =
        std::make_shared<windep::image::pe::PeImageFactory>(is_delayed);
    windep::image::ImageDependencyFactory dep_factory{image, pe_image_factory};
    auto root = dep_factory.Create();
    windep::DfsWalker<decltype(root)::element_type::Context> walker;
    auto visitor = std::make_shared<windep::image::TerminalImageTreeVisitor>(
        args["functions"].as<bool>());
    walker.Go(root, visitor);
  } catch (const std::exception &e) {
    std::cerr << "[-] " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
