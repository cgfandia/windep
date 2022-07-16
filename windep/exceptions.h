// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#pragma once
#include <Windows.h>

#include <exception>
#include <string>

#include "utils.h"

namespace windep {
namespace exc {
class WinDepException : public std::exception {
  std::string msg_;

 public:
  explicit WinDepException(const std::string& msg) : msg_(msg) {}
  const char* what() const override { return msg_.c_str(); }
};

class SeException : public WinDepException {
  unsigned int code_;
  static void __cdecl Translator(unsigned int code, PEXCEPTION_POINTERS excs) {
    throw SeException(code);
  }

 public:
  explicit SeException(unsigned int code)
      : WinDepException("Structured Exception " + utils::hex(code)),
        code_(code) {}
  unsigned int Code() const { return code_; }
  static void SetTranslator() { _set_se_translator(&SeException::Translator); }
};

class WinException : public WinDepException {
  using WinDepException::WinDepException;
};

class NotFoundException : public WinDepException {
  using WinDepException::WinDepException;
};

class ValidationException : public WinDepException {
  using WinDepException::WinDepException;
};
}  // namespace exc
}  // namespace windep
