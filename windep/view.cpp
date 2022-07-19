// copyright MIT License Copyright (c) 2022, Albert Farrakhov

#include "view.h"

#include "exceptions.h"
#include "json/json.hpp"
#include "utils.h"

namespace windep::view {
using nlohmann::json;
std::shared_ptr<View> Factory::Create(bool functions, uint8_t indent) {
  if (format_ == "ascii") {
    return std::make_shared<AsciiView>(functions, indent);
  } else if (format_ == "json") {
    return std::make_shared<JsonView>(functions, indent);
  }
  throw exc::NotFound("Unsupported format");
}

void AsciiView::Show(std::shared_ptr<Dependency<image::Image>> root,
                     std::shared_ptr<Writer> writer) {
  auto visitor =
      std::make_shared<image::AsciiTreeVisitor>(writer, functions_, indent_);
  Dfs<decltype(root)::element_type::Context> dfs{DfsDirection::kToLeaf};
  dfs.Traverse(root, visitor);
}

void JsonView::Show(std::shared_ptr<Dependency<image::Image>> root,
                    std::shared_ptr<Writer> writer) {
  auto visitor = std::make_shared<image::JsonTreeVisitor>(functions_);
  Bfs<decltype(root)::element_type::Context> bfs;
  bfs.Traverse(root, visitor);
  auto root_name = root->GetContext()->Name();
  json root_json = {{root_name, json::object()}};
  root_json[root_name]["imports"] = visitor->GetJson();
  writer->Write(root_json.dump(indent_ ? indent_ : -1) + '\n');
}
}  // namespace windep::view
