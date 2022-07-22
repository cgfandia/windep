// copyright MIT License Copyright (c) 2022, Albert Farrakhov

#pragma once
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace windep::writer {
class Writer {
 public:
  virtual void Write(const std::string&) = 0;
  virtual void Write(const std::stringstream&) = 0;
};

class StreamWriter : public Writer {
  std::shared_ptr<std::ostream> stream_;

 public:
  explicit StreamWriter(std::shared_ptr<std::ostream> stream)
      : stream_(stream) {}
  void Write(const std::string&) override;
  void Write(const std::stringstream&) override;
};

class WriterFactory {
 public:
  virtual std::shared_ptr<Writer> Create(const std::wstring& path) = 0;
};

class StreamFactory : public WriterFactory {
  std::reference_wrapper<std::ostream> std_stream_;

 public:
  explicit StreamFactory(std::ostream& std_stream = std::cout)
      : std_stream_(std_stream) {}
  std::shared_ptr<Writer> Create(const std::wstring& path) override;
};
}  // namespace windep::writer
