// copyright MIT License Copyright (c) 2022, Albert Farrakhov

#include "writer.h"

#include <iostream>

namespace windep {
void StdoutWriter::Write(const std::string& str) { std::cout << str; }
void StdoutWriter::Write(const std::stringstream& str_stream) {
  std::cout << str_stream.str();
}
}  // namespace windep
