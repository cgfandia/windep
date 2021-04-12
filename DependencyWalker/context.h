#pragma once
#include <string>


namespace dw {
class Context {
public:
  virtual std::string String() const = 0;
  virtual void Merge(std::shared_ptr<Context> other);
  virtual size_t Hash() const;
  virtual bool operator<(const Context& other) const;
  virtual bool operator==(const Context& other) const;
};

template<typename T>
struct HashShared {
  std::size_t operator()(const std::shared_ptr<T>& ctx) const {
    if (ctx) {
      return ctx->Hash();
    }
    return 0;
  }
};

template<typename T>
struct EqShared {
  bool operator()(const std::shared_ptr<T>& l,
    const std::shared_ptr<T>& r) const {
    if (l && r) {
      return *l == *r;
    }
    return l == r;
  }
};

template<typename T>
struct LtShared {
  bool operator()(const std::shared_ptr<T>& l,
    const std::shared_ptr<T>& r) const {
    if (l && r) {
      return *l < *r;
    }
    return l < r;
  }
};
} // dw namespace
