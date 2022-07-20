// copyright MIT License Copyright (c) 2022, Albert Farrakhov

#pragma once
#include <sstream>
#include <string>

namespace windep {
class Writer {
 public:
  virtual void Write(const std::string&) = 0;
  virtual void Write(const std::stringstream&) = 0;
};

class StdoutWriter : public Writer {
 public:
  void Write(const std::string&) override;
  void Write(const std::stringstream&) override;
};
}  // namespace windep
