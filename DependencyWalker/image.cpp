#include "exceptions.h"
#include "image.h"

#include <iostream>


namespace dw::image {
Image::Image(const std::string& name) : name_(name) {}


std::string Image::String() const {
  return Name();
}


void Image::Merge(std::shared_ptr<Context> other) {
  auto image = std::dynamic_pointer_cast<Image>(other);
  auto& imports = Imports();
  for (const auto& other_import : image->Imports()) {
    auto import = imports.find(other_import);
    if (import != imports.end()) {
      (*import)->Merge(other_import);
    } else {
      AddImport(other_import);
    }
  }
}


const std::string & Image::Name() const { return name_; }


const Image::ImportsCollection& Image::Imports() const { return imports_; }


void Image::AddImport(std::shared_ptr<Import> import) {
  imports_.emplace(import);
}


std::shared_ptr<Dependency<Image>> ImageDependencyFactory::CreateRecursive(const std::string & image,
                                                                           std::shared_ptr<Dependency<Image>> parent) {
  auto dependency = std::make_shared<Dependency<Image>>();
  auto image_ctx = image_factory_->Create(image);
  dependency->SetContext(image_ctx);
  dependency->AppendParent(parent);
  visited_[image] = dependency;
  for (const auto& import : image_ctx->Imports()) {
    if (import->Name() != image) {
      try {
        auto child_dep = visited_.find(import->Name());
        if (child_dep == visited_.end()) {
          auto child = CreateRecursive(import->Name(), dependency);
          child->AppendParent(dependency);
          dependency->AppendChild(child);
        } else {
          child_dep->second->AppendParent(dependency);
          dependency->AppendChild(child_dep->second);
        }
      } catch (exc::DependencyWalkerException) {}
    }
  }
  return dependency;
}


ImageDependencyFactory::ImageDependencyFactory(const std::string& root,
                                               std::shared_ptr<ImageContextFactory> image_factory) :
  root_(root), image_factory_(std::move(image_factory)) {
}


std::shared_ptr<Dependency<Image>> ImageDependencyFactory::Create() {
  return CreateRecursive(root_);
}


TerminalImageTreeVisitor::TerminalImageTreeVisitor(bool import_functions) : import_functions_(import_functions) {}


void TerminalImageTreeVisitor::Visit(std::shared_ptr<Dependency<Image>>& node, size_t height) {
  std::string offset(height, ' ');
  std::cout << offset << node->GetContext()->String() << std::endl;
  if (import_functions_) {
    for (const auto& import : node->GetContext()->Imports()) {
      for (const auto& func : import->Functions()) {
        std::cout << offset << "- " << func->String() << std::endl;
      }
    }
  }
}


Import::Import(const std::string & name) : name_(name), alias_name_(name) {
}


Import::Import(const std::string & name, const std::string & alias) : name_(name), alias_name_(alias) {
}


bool Import::operator<(const Import & other) const {
  return String() < other.String();
}


bool Import::operator==(const Import & other) const {
  return String() == other.String();
}


const std::string & Import::Name() const {
  return name_;
}


const std::string & Import::Alias() const {
  return alias_name_;
}


std::string Import::String() const {
  return Name();
}


void Import::Merge(std::shared_ptr<Context> other) {
  auto import = std::dynamic_pointer_cast<Import>(other);
  const auto& import_functions = import->Functions();
  functions_.insert(import_functions.begin(), import_functions.end());
}


const Import::FunctionsCollection& Import::Functions() const {
  return functions_;
}


void Import::AddFunction(std::shared_ptr<Function> func) {
  functions_.emplace(func);
}
} // dw::image namespace
