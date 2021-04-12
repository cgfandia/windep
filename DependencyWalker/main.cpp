#include "pe.h"
#include "walker.h"

#include <iostream>

#include <cxxopts\cxxopts.hpp>

int main(int argc, char** argv)
{
  try {
    dw::exc::SeException::SetTranslator();
    cxxopts::Options options("DependencyWalker.exe", "Windows PE format dependency resolver");
    options.positional_help("<root image>");
    options.parse_positional({ "image" });
    options.add_options()
      ("i,image", "Root image", cxxopts::value<std::string>())
      ("f,functions", "Enable functions output", cxxopts::value<bool>()->default_value("false"))
      ("d,delayed", "Enable resolving of delayed imports", cxxopts::value<bool>()->default_value("false"))
      ("h,help", "Print help", cxxopts::value<bool>()->default_value("false"))
      ;
    auto args = options.parse(argc, argv);

    if (args.count("help")) {
      std::cout << options.help() << std::endl;
      return 0;
    }

    dw::image::ImageDependencyFactory dep_factory{ 
      args["image"].as<std::string>(), 
      std::make_shared<dw::image::pe::PeImageFactory>(args["delayed"].as<bool>()) 
    };
    auto root = dep_factory.Create();
    dw::DfsWalker<decltype(root)::element_type::Context> walker;
    auto visitor = std::make_shared<dw::image::TerminalImageTreeVisitor>(args["functions"].as<bool>());
    walker.Go(root, visitor);
  } catch (const std::exception& e) {
    std::cerr << "[-] " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
