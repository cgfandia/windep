// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#pragma once
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "context.h"

namespace windep {
template <typename T>
struct Merge {
  void operator()(std::shared_ptr<T> l, std::shared_ptr<T> r) {}
};

template <typename T>
struct Hash {
  size_t operator()(const T& ctx) const { return std::hash<T>()(ctx); }
};

template <typename T, typename Hasher = Hash<T>, typename Merger = Merge<T>>
class Dependency {
  struct HashShared {
    std::size_t operator()(const std::shared_ptr<Dependency<T>>& dep) const {
      if (dep) {
        return Hasher()(*dep->GetContext());
      }
      return 0;
    }
  };

  struct CompareShared {
    bool operator()(const std::shared_ptr<Dependency<T>>& l,
                    const std::shared_ptr<Dependency<T>>& r) const {
      if (l && r) {
        return *l == *r;
      }
      return l == r;
    }
  };

 public:
  using Context = T;
  using ParentsCollection = std::vector<std::weak_ptr<Dependency>>;
  using ChildrenCollection =
      std::unordered_set<std::shared_ptr<Dependency>, Dependency::HashShared,
                         Dependency::CompareShared>;

 protected:
  ParentsCollection parents_;
  ChildrenCollection children_;
  std::shared_ptr<T> context_;

 public:
  const ParentsCollection& Parents() const { return parents_; }
  const ChildrenCollection& Children() const { return children_; }
  void AppendChild(std::shared_ptr<Dependency> child) {
    if (child) {
      auto old_child_it = children_.find(child);
      if (old_child_it != children_.end()) {
        if ((*old_child_it)->GetContext() && child->GetContext()) {
          Merger()((*old_child_it)->GetContext(), child->GetContext());
        }
      } else {
        children_.emplace(child);
      }
    }
  }
  void AppendParent(std::shared_ptr<Dependency> parent) {
    if (parent) {
      parents_.push_back(parent);
    }
  }
  void SetContext(std::shared_ptr<T> context) { context_ = context; }
  std::shared_ptr<Dependency> Clone() const {
    auto clone = std::make_shared<Dependency>(*this);
    auto context = GetContext();
    if (context) {
      clone.SetContext(std::make_shared<T>(*context));
    }
    return clone;
  }
  std::shared_ptr<T> GetContext() const { return context_; }
  bool IsLeaf() const { return Children().empty(); }
  bool operator<(const Dependency<T>& other) const {
    if (GetContext() && other.GetContext()) {
      return *GetContext() < *other.GetContext();
    }
    return GetContext() < other.GetContext();
  }
  bool operator==(const Dependency<T>& other) const {
    if (GetContext() && other.GetContext()) {
      return *GetContext() == *other.GetContext();
    }
    return GetContext() == other.GetContext();
  }
};

template <typename T>
class DependencyFactory {
  virtual std::shared_ptr<Dependency<T>> Create() = 0;
};
}  // namespace windep
