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
  } else if (format_ == "dot") {
    return std::make_shared<DotView>(indent);
  } else if (format_ == "csv") {
    return std::make_shared<CsvView>();
  }
  throw exc::NotFound("Unsupported format");
}

void AsciiView::Show(std::shared_ptr<Dependency<image::Image>> root,
                     std::shared_ptr<writer::Writer> writer) {
  auto visitor =
      std::make_shared<image::AsciiTreeVisitor>(writer, functions_, indent_);
  Dfs<decltype(root)::element_type::Context> dfs{DfsDirection::kToLeaf};
  dfs.Traverse(root, visitor);
}

void JsonView::Show(std::shared_ptr<Dependency<image::Image>> root,
                    std::shared_ptr<writer::Writer> writer) {
  auto visitor = std::make_shared<image::JsonTreeVisitor>(functions_);
  Bfs<decltype(root)::element_type::Context> bfs;
  bfs.Traverse(root, visitor);
  auto root_name = root->GetContext()->Name();
  json root_json = {{root_name, json::object()}};
  root_json[root_name]["imports"] = visitor->Json();
  writer->Write(root_json.dump(indent_ ? indent_ : -1) + '\n');
}

void DotView::Show(std::shared_ptr<Dependency<image::Image>> root,
                   std::shared_ptr<writer::Writer> writer) {
  auto visitor = std::make_shared<image::DotTreeVisitor>(indent_);
  Bfs<decltype(root)::element_type::Context> bfs;
  bfs.Traverse(root, visitor);
  writer->Write(visitor->Dot());
}

void CsvView::Show(std::shared_ptr<Dependency<image::Image>> root,
                   std::shared_ptr<writer::Writer> writer) {
  auto visitor = std::make_shared<image::CsvTreeVisitor>();
  Bfs<decltype(root)::element_type::Context> bfs;
  bfs.Traverse(root, visitor);
  writer->Write(visitor->Csv());
}
}  // namespace windep::view
