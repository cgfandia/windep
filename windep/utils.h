// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#pragma once
#include <iomanip>
#include <sstream>
#include <string>

namespace windep::utils {
std::string lower(std::string text);
bool startswith(const std::string& text, const std::string& prefix);
std::string w2a(const std::wstring& wide);
std::wstring a2w(const std::string& ansii);
template <typename T>
std::string hex(T i) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex
         << i;
  return stream.str();
}
}  // namespace windep::utils
