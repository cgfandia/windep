#pragma once
#include "dependency.h"
#include "context.h"

#include <unordered_set>


namespace dw {
template<typename T>
class TreeVisitor {
public:
  virtual void Visit(std::shared_ptr<Dependency<T>>& node, size_t height) = 0;
};

template<typename T>
class Walker {
public:
  virtual void Go(std::shared_ptr<Dependency<T>> root, std::shared_ptr<TreeVisitor<T>> visitor) = 0;
};

template<typename T>
class DfsWalker : public Walker<T> {
  size_t height_ = 0;
  std::unordered_set<std::shared_ptr<Dependency<T>>> visited_;
public:
  void Go(std::shared_ptr<Dependency<T>> root, std::shared_ptr<TreeVisitor<T>> visitor) override {
    visitor->Visit(root, height_);
    if (!visited_.count(root)) {  // Exclude infinite recursion due to cyclic dependency
      visited_.emplace(root);
      height_++;
      for (auto& child : root->Children()) {
        Go(child, visitor);
      }
      height_--;
    }
  }
};
} // dw namespace