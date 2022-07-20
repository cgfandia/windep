// copyright MIT License Copyright (c) 2022, Albert Farrakhov

#pragma once
#include <memory>
#include <string>

#include "dependency.h"
#include "image.h"
#include "utils.h"
#include "writer.h"

namespace windep::view {
class View {
 public:
  virtual void Show(std::shared_ptr<Dependency<image::Image>>,
                    std::shared_ptr<Writer>) = 0;
};

class Factory {
  std::string format_;

 public:
  explicit Factory(const std::string& format) : format_(utils::lower(format)) {}
  std::shared_ptr<View> Create(bool functions = false, uint8_t indent = 2);
};

class AsciiView : public View {
  bool functions_;
  uint8_t indent_;

 public:
  explicit AsciiView(bool functions, uint8_t indent)
      : functions_(functions), indent_(indent) {}
  void Show(std::shared_ptr<Dependency<image::Image>>,
            std::shared_ptr<Writer>) override;
};

class JsonView : public View {
  bool functions_;
  uint8_t indent_;

 public:
  explicit JsonView(bool functions, uint8_t indent)
      : functions_(functions), indent_(indent) {}
  void Show(std::shared_ptr<Dependency<image::Image>>,
            std::shared_ptr<Writer>) override;
};

class DotView : public View {
  uint8_t indent_;

 public:
  explicit DotView(uint8_t indent = 2) : indent_(indent) {}
  void Show(std::shared_ptr<Dependency<image::Image>>,
            std::shared_ptr<Writer>) override;
};
}  // namespace windep::view
