// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#include <filesystem>
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

class NullWriter : public windep::writer::Writer {
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

TEST_CASE("matrix", "[view,stdout,file,null]") {
  const std::string binary = "kernel32.dll";
  auto root = CreateTree(binary, false);
  auto root_delayed = CreateTree(binary, true);
  auto ascii_view = windep::view::Factory{"ascii"}.Create(true, 2);
  auto json_view = windep::view::Factory{"json"}.Create(true, 2);
  auto dot_view = windep::view::Factory{"dot"}.Create();
  auto csv_view = windep::view::Factory{"csv"}.Create();
  auto stdout_writer = windep::writer::StreamFactory().Create(L"");
  auto tmp_file = std::filesystem::temp_directory_path() / "windep_file_output";
  auto file_writer = windep::writer::StreamFactory().Create(tmp_file.wstring());
  auto null_writer = std::make_shared<NullWriter>();
  for (auto view : {ascii_view, json_view, dot_view, csv_view}) {
    for (auto writer : {stdout_writer, file_writer}) {
      REQUIRE_NOTHROW(view->Show(root, writer));
    }
    REQUIRE_NOTHROW(view->Show(root_delayed, null_writer));
  }
  REQUIRE(std::filesystem::file_size(tmp_file) > 0);
}
