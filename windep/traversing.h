// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#pragma once
#include <deque>
#include <memory>
#include <unordered_set>
#include <utility>

#include "context.h"
#include "dependency.h"

namespace windep {
template <typename T>
class TreeVisitor {
 public:
  virtual void Visit(std::shared_ptr<Dependency<T>> node, size_t height) = 0;
};

template <typename T>
class TraversalStrategy {
 public:
  virtual void Traverse(std::shared_ptr<Dependency<T>> root,
                        std::shared_ptr<TreeVisitor<T>> visitor) = 0;
};

enum DfsDirection : uint8_t { kToLeaf, kFromLeaf };
template <typename T>
class Dfs : public TraversalStrategy<T> {
  size_t height_ = 0;
  std::unordered_set<std::shared_ptr<Dependency<T>>> visited_;
  DfsDirection direction_;

 public:
  explicit Dfs(DfsDirection direction = DfsDirection::kToLeaf)
      : direction_(direction) {}
  void Traverse(std::shared_ptr<Dependency<T>> root,
                std::shared_ptr<TreeVisitor<T>> visitor) override {
    if (direction_ == DfsDirection::kToLeaf) visitor->Visit(root, height_);
    // Avoid infinite recursion due to the cyclic dependency
    if (!visited_.count(root)) {
      visited_.insert(root);
      height_++;
      for (auto child : root->Children()) {
        Traverse(child, visitor);
      }
      if (height_) height_--;
      if (direction_ == DfsDirection::kFromLeaf) visitor->Visit(root, height_);
    }
  }
};

template <typename T>
class Bfs : public TraversalStrategy<T> {
  std::unordered_set<std::shared_ptr<Dependency<T>>> visited_;
  std::deque<std::pair<std::shared_ptr<Dependency<T>>, size_t>> queue_;
  std::shared_ptr<TreeVisitor<T>> visitor_;
  /*
    Each node can have more than one parent, including link to self.
    In this case we have to check parents of each child.
    Possible graph:
        A
       /
      A   B
     / \ /
    C   D
  */
  void BfsTraverse() {
    while (queue_.size()) {
      auto [node, height] = queue_.front();
      queue_.pop_front();
      // Avoid infinite recursion due to the cyclic dependency
      if (!visited_.count(node)) {
        visited_.insert(node);
        visitor_->Visit(node, height);
        for (auto child : node->Children()) {
          queue_.emplace_back(child, height + 1);
        }
        for (auto parent : node->Parents()) {
          if (!parent.expired()) {
            queue_.emplace_back(parent.lock(), height ? height - 1 : 0);
          }
        }
      }
    }
  }

 public:
  void Traverse(std::shared_ptr<Dependency<T>> root,
                std::shared_ptr<TreeVisitor<T>> visitor) override {
    queue_.clear();
    queue_.emplace_back(root, 0);
    visitor_ = visitor;
    BfsTraverse();
  }
};
}  // namespace windep
