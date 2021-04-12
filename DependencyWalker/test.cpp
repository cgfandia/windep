#include "pch.h"
#include "../DependencyWalker/pe.h"

TEST(TestPeImg, TestNonEmptyDependencies) {
  dw::image::pe::PeImage image{ "kernel32.dll" };
  ASSERT_NO_THROW(image.Parse());
  EXPECT_GT(image.Imports().size(), 0);
  EXPECT_GT(image.ImportFunctions().size(), 0);
}

TEST(TestPeImg, TestEmptyDependencies) {
  dw::image::pe::PeImage image{ "ntdll.dll" };
  ASSERT_NO_THROW(image.Parse());
  EXPECT_EQ(image.Imports().size(), 0);
  EXPECT_EQ(image.ImportFunctions().size(), 0);
}

TEST(TestPeImg, TestNoImage) {
  dw::image::pe::PeImage image{ "unknown_image_name.dll" };
  ASSERT_THROW(image.Parse(), dw::exc::NotFoundException);
}

TEST(TestPeDependency, TestExplorerExe) {
  auto dep_factory = dw::image::ImageDependencyFactory{ "explorer.exe", std::make_shared<dw::image::pe::PeImageFactory>() };
  auto root_dep = dep_factory.Create();
  EXPECT_GT(root_dep->Children().size(), 0);
}

TEST(TestPeDependency, TestNtdll) {
  auto dep_factory = dw::image::ImageDependencyFactory{ "ntdll.dll", std::make_shared<dw::image::pe::PeImageFactory>() };
  auto root_dep = dep_factory.Create();
  EXPECT_EQ(root_dep->Children().size(), 0);
}

TEST(TestPe, TestVersionlessRegex) {
  auto versionless = dw::image::pe::VersionlessDllName(L"api-ms-onecoreuap-print-render-l1-1-0");
  EXPECT_EQ(versionless, L"api-ms-onecoreuap-print-render");
}