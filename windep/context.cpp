// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#include "context.h"

#include "exceptions.h"

namespace windep {
void Context::Merge(std::shared_ptr<Context> other) {}

size_t Context::Hash() const { return std::hash<std::string>()(String()); }

bool Context::operator<(const Context& other) const {
  return String() < other.String();
}

bool Context::operator==(const Context& other) const {
  return String() == other.String();
}
}  // namespace windep
