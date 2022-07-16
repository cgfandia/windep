// MIT License

// Copyright (c) 2021 Albert

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "pe.h"

TEST(TestPeImg, TestNonEmptyDependencies) {
  windep::image::pe::PeImage image{"kernel32.dll"};
  ASSERT_NO_THROW(image.Parse());
  EXPECT_GT(image.Imports().size(), 0);
  EXPECT_GT(image.ImportFunctions().size(), 0);
}

TEST(TestPeImg, TestEmptyDependencies) {
  windep::image::pe::PeImage image{"ntdll.dll"};
  ASSERT_NO_THROW(image.Parse());
  EXPECT_EQ(image.Imports().size(), 0);
  EXPECT_EQ(image.ImportFunctions().size(), 0);
}

TEST(TestPeImg, TestNoImage) {
  windep::image::pe::PeImage image{"unknown_image_name.dll"};
  ASSERT_THROW(image.Parse(), windep::exc::NotFoundException);
}

TEST(TestPeDependency, TestExplorerExe) {
  auto dep_factory = windep::image::ImageDependencyFactory{
      "explorer.exe", std::make_shared<windep::image::pe::PeImageFactory>()};
  auto root_dep = dep_factory.Create();
  EXPECT_GT(root_dep->Children().size(), 0);
}

TEST(TestPeDependency, TestNtdll) {
  auto dep_factory = windep::image::ImageDependencyFactory{
      "ntdll.dll", std::make_shared<windep::image::pe::PeImageFactory>()};
  auto root_dep = dep_factory.Create();
  EXPECT_EQ(root_dep->Children().size(), 0);
}

TEST(TestPe, TestVersionlessRegex) {
  auto versionless = windep::image::pe::VersionlessDllName(
      L"api-ms-onecoreuap-print-render-l1-1-0");
  EXPECT_EQ(versionless, L"api-ms-onecoreuap-print-render");
}
