#pragma once
#include "context.h"
#include "dependency.h"
#include "walker.h"

#include <set>
#include <unordered_map>


namespace dw {
namespace image {
class Function : public Context {
};


class Import : public Context {
public:
  using FunctionsCollection = std::set<std::shared_ptr<Function>, LtShared<Function>>;
protected:
  std::string name_;
  std::string alias_name_;
  FunctionsCollection functions_;
public:
  Import(const std::string& name);
  explicit Import(const std::string& name, const std::string& alias);
  bool operator<(const Import& other) const;
  bool operator==(const Import& other) const;
  virtual const std::string& Name() const;
  virtual const std::string& Alias() const;
  virtual std::string String() const override;
  virtual void Merge(std::shared_ptr<Context> other) override;
  virtual const FunctionsCollection& Functions() const;
  virtual void AddFunction(std::shared_ptr<Function> func);
};


class Image : public Context {
public:
  using ImportsCollection = std::set<std::shared_ptr<Import>, LtShared<Import>>;
protected:
  std::string name_;
  ImportsCollection imports_;
public:
  Image(const std::string& name);
  std::string String() const override;
  virtual void Merge(std::shared_ptr<Context> other) override;
  virtual void Parse() = 0;
  virtual const std::string& Name() const;
  virtual const ImportsCollection& Imports() const;
  void AddImport(std::shared_ptr<Import> import);
};


class ImageContextFactory {
public:
  virtual std::shared_ptr<Image> Create(const std::string& image) = 0;
};


class ImageDependencyFactory : public DependencyFactory<Image> {
  std::string root_;
  std::shared_ptr<ImageContextFactory> image_factory_;
  std::unordered_map<std::string, std::shared_ptr<Dependency<Image>>> visited_;
  std::shared_ptr<Dependency<Image>> CreateRecursive(const std::string& image, std::shared_ptr<Dependency<Image>> parent = nullptr);
public:
  explicit ImageDependencyFactory(const std::string& root, std::shared_ptr<ImageContextFactory> image_factory);
  virtual std::shared_ptr<Dependency<Image>> Create() override;
};


class ImageTreeVisitor : public TreeVisitor<Image> {
public:
  virtual void Visit(std::shared_ptr<Dependency<Image>>& node, size_t height) = 0;
};


class TerminalImageTreeVisitor : public TreeVisitor<Image> {
  bool import_functions_ = false;
public:
  TerminalImageTreeVisitor(bool import_functions = false);
  void Visit(std::shared_ptr<Dependency<Image>>& node, size_t height) override;
};
}


template<>
struct Merge<image::Image> {
  void operator()(std::shared_ptr<image::Image> l, std::shared_ptr<image::Image> r) {
    l->Merge(r);
  }
};


template<>
struct Hash<image::Image> {
  size_t operator()(const image::Image& ctx) const {
    return ctx.Hash();
  }
};
} // dw::image namespace
