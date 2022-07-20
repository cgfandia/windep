// copyright MIT License Copyright (c) 2021, Albert Farrakhov
#include <memory>

#include "catch2/catch_amalgamated.hpp"
#include "context.h"
#include "dependency.h"
#include "exceptions.h"
#include "image.h"
#include "pe.h"
#include "traversing.h"
#include "utils.h"
#include "view.h"
#include "writer.h"

class NullWriter : public windep::Writer {
  void Write(const std::string& str) override {}
  void Write(const std::stringstream& str_stream) override {}
};

TEST_CASE("with_imports", "[image]") {
  windep::image::pe::PeImage image{"kernel32.dll", true};
  REQUIRE_NOTHROW(image.Parse());
  REQUIRE(image.Imports().size() > 0);
}

TEST_CASE("without_imports", "[image]") {
  windep::image::pe::PeImage image{"ntdll.dll", true};
  REQUIRE_NOTHROW(image.Parse());
  REQUIRE(image.Imports().size() == 0);
}

TEST_CASE("not_found", "[image]") {
  windep::image::pe::PeImage image{"unknown_image_name.dll", true};
  REQUIRE_THROWS_AS(image.Parse(), windep::exc::NotFound);
  try {
    image.Parse();
  } catch (windep::exc::NotFound& e) {
    e.what();
  }
}

TEST_CASE("versionless", "[image]") {
  auto& pe_meta = windep::image::pe::PeMeta::Instance();
  const auto versionless =
      pe_meta.VersionlessDllName(L"api-ms-onecoreuap-print-render-l1-1-0");
  REQUIRE(versionless == L"api-ms-onecoreuap-print-render");
}

TEST_CASE("with_dependencies", "[dependencies]") {
  auto img_fc = std::make_shared<windep::image::pe::PeImageFactory>();
  auto dep_fc = windep::image::ImageDependencyFactory{"explorer.exe", img_fc};
  auto root_dep = dep_fc.Create();
  REQUIRE(root_dep->Children().size() > 0);
}

TEST_CASE("without_dependencies", "[dependencies]") {
  auto img_fc = std::make_shared<windep::image::pe::PeImageFactory>();
  auto dep_fc = windep::image::ImageDependencyFactory{"ntdll.dll", img_fc};
  auto root_dep = dep_fc.Create();
  REQUIRE(root_dep->Children().size() == 0);
}

std::shared_ptr<windep::Dependency<windep::image::Image>> CreateTree(
    const std::string& binary, bool delayed = false) {
  const auto img_fc =
      std::make_shared<windep::image::pe::PeImageFactory>(delayed);
  windep::image::ImageDependencyFactory dep_factory{binary, img_fc};
  return dep_factory.Create();
}

TEST_CASE("non_delayed", "[view,stdout]") {
  auto root = CreateTree("kernel32.dll", false);
  auto stdout_writer = std::make_shared<windep::StdoutWriter>();
  auto ascii_view = windep::view::Factory{"ascii"}.Create(true, 2);
  auto json_view = windep::view::Factory{"json"}.Create(true, 2);
  auto dot_view = windep::view::Factory{"dot"}.Create();
  auto csv_view = windep::view::Factory{"csv"}.Create();
  REQUIRE_NOTHROW(ascii_view->Show(root, stdout_writer));
  REQUIRE_NOTHROW(json_view->Show(root, stdout_writer));
  REQUIRE_NOTHROW(dot_view->Show(root, stdout_writer));
  REQUIRE_NOTHROW(csv_view->Show(root, stdout_writer));
}

TEST_CASE("delayed", "[view,null]") {
  auto root = CreateTree("kernel32.dll", true);
  auto null = std::make_shared<NullWriter>();
  auto ascii_view = windep::view::Factory{"ascii"}.Create(true, 2);
  auto json_view = windep::view::Factory{"json"}.Create(true, 2);
  auto dot_view = windep::view::Factory{"dot"}.Create();
  auto csv_view = windep::view::Factory{"csv"}.Create();
  REQUIRE_NOTHROW(ascii_view->Show(root, null));
  REQUIRE_NOTHROW(json_view->Show(root, null));
  REQUIRE_NOTHROW(csv_view->Show(root, null));
}
