#pragma once
#include <exception>
#include <string>
#include <Windows.h>

#include "utils.h"


namespace dw {
namespace exc {
class DependencyWalkerException : public std::exception {
  std::string msg_;
public:
  DependencyWalkerException(const std::string& msg) : msg_(msg) {}
  const char* what() const override { return msg_.c_str(); }
}; 

class SeException : public DependencyWalkerException {
  unsigned int code_;
  static void __cdecl Translator(unsigned int code, PEXCEPTION_POINTERS excs) {
    throw SeException(code);
  }
public:
  explicit SeException(unsigned int code) : code_(code), DependencyWalkerException("Structured Exception " + utils::hex(code)) {
  }
  unsigned int Code() const {
    return code_;
  }
  static void SetTranslator() {
    _set_se_translator(&SeException::Translator);
  }
};

class WinException : public DependencyWalkerException {
  using DependencyWalkerException::DependencyWalkerException;
};

class NotFoundException : public DependencyWalkerException {
  using DependencyWalkerException::DependencyWalkerException;
};

class ValidationException : public DependencyWalkerException {
  using DependencyWalkerException::DependencyWalkerException;
};
} // exc namespace
} // dw namespace
