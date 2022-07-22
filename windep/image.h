// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#pragma once
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "context.h"
#include "dependency.h"
#include "json/json.hpp"
#include "traversing.h"
#include "writer.h"

namespace windep {
namespace image {
using nlohmann::json;
class Function : public Context {
 public:
  virtual const std::string& Name() const = 0;
};

class Import : public Context {
 public:
  using FunctionsCollection =
      std::set<std::shared_ptr<Function>, LtShared<Function>>;

 protected:
  std::string name_;
  std::string alias_name_;
  FunctionsCollection functions_;
  bool unresolved_ = false;

 public:
  explicit Import(const std::string& name);
  explicit Import(const std::string& name, const std::string& alias);
  bool operator<(const Import& other) const;
  bool operator==(const Import& other) const;
  virtual const std::string& Name() const;
  virtual const std::string& Alias() const;
  virtual bool IsUnresolved() const;
  std::string String() const override;
  void Merge(std::shared_ptr<Context> other) override;
  virtual const FunctionsCollection& Functions() const;
  virtual void AddFunction(std::shared_ptr<Function> func);
  virtual void SetUnresolved(bool enable);
};

class Image : public Context {
 public:
  using ImportsCollection = std::set<std::shared_ptr<Import>, LtShared<Import>>;

 protected:
  std::string name_;
  ImportsCollection imports_;
  std::filesystem::path path_;

 public:
  explicit Image(const std::string& name);
  std::string String() const override;
  void Merge(std::shared_ptr<Context> other) override;
  virtual void Parse() = 0;
  virtual const std::string& Name() const;
  virtual const ImportsCollection& Imports() const;
  void AddImport(std::shared_ptr<Import> import);
  void ClearImports();
  const std::filesystem::path& Path() const;
  void SetPath(const std::wstring& path);
};

class ImageContextFactory {
 public:
  virtual std::shared_ptr<Image> Create(const std::string& image) = 0;
};

class ImageDependencyFactory : public DependencyFactory<Image> {
  std::string root_;
  std::shared_ptr<ImageContextFactory> image_factory_;
  std::unordered_map<std::string, std::shared_ptr<Dependency<Image>>> visited_;
  std::shared_ptr<Dependency<Image>> CreateRecursive(
      const std::string& image,
      std::shared_ptr<Dependency<Image>> parent = nullptr);

 public:
  explicit ImageDependencyFactory(
      const std::string& root,
      std::shared_ptr<ImageContextFactory> image_factory);
  std::shared_ptr<Dependency<Image>> Create() override;
};

class ImageTreeVisitor : public TreeVisitor<Image> {
 public:
  virtual void Visit(std::shared_ptr<Dependency<Image>> node,
                     size_t height) = 0;
};

class AsciiTreeVisitor : public TreeVisitor<Image> {
  std::shared_ptr<writer::Writer> writer_;
  bool functions_ = false;
  uint8_t indent_;

 public:
  explicit AsciiTreeVisitor(std::shared_ptr<writer::Writer> writer,
                            bool functions = false, uint8_t indent = 2);
  void Visit(std::shared_ptr<Dependency<Image>> node, size_t height) override;
};

class JsonTreeVisitor : public TreeVisitor<Image> {
  json json_;
  bool functions_ = false;

 public:
  explicit JsonTreeVisitor(bool functions = false);
  void Visit(std::shared_ptr<Dependency<Image>> node, size_t height) override;
  json& Json();
};

class DotTreeVisitor : public TreeVisitor<Image> {
  std::string statements_;
  uint8_t indent_;
  std::string FormatId(std::shared_ptr<Context> ctx) const;

 public:
  explicit DotTreeVisitor(uint8_t indent = 2) : indent_(indent) {}
  void Visit(std::shared_ptr<Dependency<Image>> node, size_t height) override;
  std::string Dot();
};

class CsvTreeVisitor : public TreeVisitor<Image> {
  std::string lines_;

 public:
  void Visit(std::shared_ptr<Dependency<Image>> node, size_t height) override;
  std::string Csv();
};
}  // namespace image

template <>
struct Merge<image::Image> {
  void operator()(std::shared_ptr<image::Image> l,
                  std::shared_ptr<image::Image> r) {
    l->Merge(r);
  }
};

template <>
struct Hash<image::Image> {
  size_t operator()(const image::Image& ctx) const { return ctx.Hash(); }
};
}  // namespace windep
