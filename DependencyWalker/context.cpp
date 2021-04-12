#include "context.h"
#include "exceptions.h"


namespace dw {
void Context::Merge(std::shared_ptr<Context> other) {
}


size_t Context::Hash() const {
  return std::hash<std::string>()(String());
}


bool Context::operator<(const Context& other) const {
  return String() < other.String();
}


bool Context::operator==(const Context& other) const {
  return String() == other.String();
}
} // dw namespace