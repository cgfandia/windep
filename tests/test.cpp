// copyright MIT License Copyright (c) 2021, Albert Farrakhov
#include <memory>

#include "catch2/catch_amalgamated.hpp"
#include "context.h"
#include "dependency.h"
#include "exceptions.h"
#include "image.h"
#include "pe.h"
#include "utils.h"
#include "walker.h"

TEST_CASE("with imports", "[image]") {
  windep::image::pe::PeImage image{"kernel32.dll", true};
  REQUIRE_NOTHROW(image.Parse());
  REQUIRE(image.Imports().size() > 0);
}

TEST_CASE("without imports", "[image]") {
  windep::image::pe::PeImage image{"ntdll.dll", true};
  REQUIRE_NOTHROW(image.Parse());
  REQUIRE(image.Imports().size() == 0);
}

TEST_CASE("not found", "[image]") {
  windep::image::pe::PeImage image{"unknown_image_name.dll", true};
  REQUIRE_THROWS_AS(image.Parse(), windep::exc::NotFoundException);
}

TEST_CASE("versionless", "[image]") {
  auto& pe_meta = windep::image::pe::PeMeta::Instance();
  const auto versionless =
      pe_meta.VersionlessDllName(L"api-ms-onecoreuap-print-render-l1-1-0");
  REQUIRE(versionless == L"api-ms-onecoreuap-print-render");
}

TEST_CASE("with dependencies", "[dependencies]") {
  auto img_fc = std::make_shared<windep::image::pe::PeImageFactory>();
  auto dep_fc = windep::image::ImageDependencyFactory{"explorer.exe", img_fc};
  auto root_dep = dep_fc.Create();
  REQUIRE(root_dep->Children().size() > 0);
}

TEST_CASE("without dependencies", "[dependencies]") {
  auto img_fc = std::make_shared<windep::image::pe::PeImageFactory>();
  auto dep_fc = windep::image::ImageDependencyFactory{"ntdll.dll", img_fc};
  auto root_dep = dep_fc.Create();
  REQUIRE(root_dep->Children().size() == 0);
}
