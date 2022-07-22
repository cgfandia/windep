// copyright MIT License Copyright (c) 2022, Albert Farrakhov

#include "writer.h"

#include <fstream>
#include <iostream>

namespace windep::writer {
void StreamWriter::Write(const std::string& str) { *stream_ << str; }
void StreamWriter::Write(const std::stringstream& str_stream) {
  *stream_ << str_stream.str();
}

std::shared_ptr<Writer> StreamFactory::Create(const std::wstring& path) {
  if (path.empty()) {
    std::shared_ptr<std::ostream> stream_ptr(&std_stream_.get(), [](void*) {});
    return std::make_shared<StreamWriter>(stream_ptr);
  } else {
    auto stream_ptr = std::make_shared<std::fstream>(path, std::fstream::out);
    return std::make_shared<StreamWriter>(stream_ptr);
  }
}
}  // namespace windep::writer
