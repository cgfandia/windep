// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#include "utils.h"

#include <Windows.h>

#include <algorithm>
#include <cctype>

#include "exceptions.h"

namespace windep::utils {
std::string lower(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return text;
}

bool startswith(const std::string& text, const std::string& prefix) {
  return !text.compare(0, prefix.size(), prefix);
}

std::string w2a(const std::wstring& wide) {
  auto required_size =
      ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(),
                            static_cast<int>(wide.size()), nullptr, 0, 0, 0);
  if (!required_size) {
    auto last_error = ::GetLastError();
    if (last_error == ERROR_NO_UNICODE_TRANSLATION) {
      throw exc::ValidationException(
          "Unable to translate wide char string to ansii");
    }
    return "";
  }
  std::string ansii(required_size, '\0');
  ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(),
                        static_cast<int>(wide.size()), ansii.data(),
                        static_cast<int>(ansii.size()), 0, 0);
  return ansii;
}

std::wstring a2w(const std::string& ansii) {
  auto required_size =
      ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ansii.data(),
                            static_cast<int>(ansii.size()), nullptr, 0);
  if (!required_size) {
    auto last_error = ::GetLastError();
    if (last_error == ERROR_NO_UNICODE_TRANSLATION) {
      throw exc::ValidationException(
          "Unable to translate ansii to wide char string");
    }
    return L"";
  }
  std::wstring wide(required_size, L'\0');
  ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ansii.data(),
                        static_cast<int>(ansii.size()), wide.data(),
                        static_cast<int>(wide.size()));
  return wide;
}
}  // namespace windep::utils
