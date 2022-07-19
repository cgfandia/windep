// copyright MIT License Copyright (c) 2021, Albert Farrakhov
#include "pe.h"

#include <utility>

#include "exceptions.h"
#include "utils.h"

namespace windep::image::pe {
API_SET_NAMESPACE_ARRAY* GetApiSetHeader() {
  static pfnNtQueryInformationProcess NtQueryInformationProcess =
      reinterpret_cast<pfnNtQueryInformationProcess>(GetProcAddress(
          LoadLibraryW(L"ntdll.dll"), "NtQueryInformationProcess"));
  if (!NtQueryInformationProcess) return NULL;

  PROCESS_BASIC_INFORMATION info;
  if (NtQueryInformationProcess(GetCurrentProcess(), 0, &info, sizeof(info),
                                NULL) != S_OK) {
    return NULL;
  }

#if defined(_WIN64)
  return reinterpret_cast<API_SET_NAMESPACE_ARRAY*>(
      (reinterpret_cast<PEB64*>(info.PebBaseAddress))->ApiSet);
#elif defined(_WIN32)
  return reinterpret_cast<API_SET_NAMESPACE_ARRAY*>(
      (reinterpret_cast<PEB32*>(info.PebBaseAddress))->ApiSet);
#else
  return NULL;  // unsupported architecture
#endif
}

void PeImage::Parse() {
  LoadedImage loaded_image{name_};
  auto imports = ParseImports(loaded_image);
  if (delayed_) {
    auto delayed_imports = ParseDelayedImports(loaded_image);
    imports.insert(delayed_imports.begin(), delayed_imports.end());
  }
  ClearImports();
  for (auto import : imports) {
    AddImport(import);
  }
}

LoadedImage::LoadedImage(const std::string& name) {
  image_handle_ =
      ::LoadLibraryExA(name.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
  if (!image_handle_) {
    throw exc::NotFound("Cannot open '" + name + "' image");
  }
  image_view_ =
      reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(image_handle_) &
                               (static_cast<uintptr_t>(-1) << 1));
  dos_header_ = static_cast<PIMAGE_DOS_HEADER>(image_view_);
  if (dos_header_->e_magic != IMAGE_DOS_SIGNATURE) {
    throw exc::Validation("Image '" + name + "' is not executable");
  }
  nt_headers_.x32 = reinterpret_cast<PIMAGE_NT_HEADERS32>(
      static_cast<PBYTE>(image_view_) + dos_header_->e_lfanew);
  if (IsPe64()) {
    nt_headers_.x64 = reinterpret_cast<PIMAGE_NT_HEADERS64>(
        static_cast<PBYTE>(image_view_) + dos_header_->e_lfanew);
  }
  section_headers_ = IMAGE_FIRST_SECTION(nt_headers_.x32);
}

Image::ImportsCollection PeImage::ParseImports(const LoadedImage& img) const {
  Image::ImportsCollection imports;
  auto section = img.DataDirectory()[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (section.Size) {
    auto descr = img.Read<PIMAGE_IMPORT_DESCRIPTOR>(section.VirtualAddress);
    while (descr->OriginalFirstThunk && !(descr->OriginalFirstThunk & 1)) {
      auto virtual_name = img.Read<PCSTR>(descr->Name);
      if (virtual_name) {
        auto logic_name = PeMeta::Instance().VirtualToLogic(virtual_name);
        auto import = std::make_shared<PeImport>(logic_name, virtual_name);
        const auto find_import_funcs =
            [&import, &img, this](auto thunk, const auto ordinal_flag) {
              if (thunk) {
                while (thunk->u1.AddressOfData) {
                  if (!(thunk->u1.Ordinal & ordinal_flag)) {
                    auto func_meta = img.Read<PIMAGE_IMPORT_BY_NAME>(
                        thunk->u1.AddressOfData);
                    if (func_meta) {
                      import->AddFunction(std::make_shared<PeFunction>(
                          func_meta->Name, import));
                    }
                  }
                  thunk++;
                }
              }
            };
        if (img.IsPe64()) {
          find_import_funcs(
              img.Read<PIMAGE_THUNK_DATA64>(descr->OriginalFirstThunk),
              IMAGE_ORDINAL_FLAG64);
        } else {
          find_import_funcs(
              img.Read<PIMAGE_THUNK_DATA32>(descr->OriginalFirstThunk),
              IMAGE_ORDINAL_FLAG32);
        }
        imports.insert(import);
      }
      descr++;
    }
  }
  return imports;
}

Image::ImportsCollection PeImage::ParseDelayedImports(
    const LoadedImage& img) const {
  Image::ImportsCollection imports;
  auto section = img.DataDirectory()[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
  if (section.Size) {
    auto descr = img.Read<PIMAGE_DELAYLOAD_DESCRIPTOR>(section.VirtualAddress);
    while (descr->DllNameRVA) {
      auto virtual_name = img.Read<PCSTR>(descr->DllNameRVA);
      if (virtual_name) {
        auto logic_name = PeMeta::Instance().VirtualToLogic(virtual_name);
        auto import = std::make_shared<PeImport>(logic_name, virtual_name);
        const auto find_import_funcs =
            [&import, &img, this](auto thunk, const auto ordinal_flag) {
              if (thunk) {
                while (thunk->u1.AddressOfData) {
                  if (!(thunk->u1.Ordinal & ordinal_flag)) {
                    auto func_meta = img.Read<PIMAGE_IMPORT_BY_NAME>(
                        thunk->u1.AddressOfData);
                    if (func_meta) {
                      import->AddFunction(std::make_shared<PeFunction>(
                          func_meta->Name, import));
                    }
                  }
                  thunk++;
                }
              }
            };
        if (img.IsPe64()) {
          find_import_funcs(
              img.Read<PIMAGE_THUNK_DATA64>(descr->ImportNameTableRVA),
              IMAGE_ORDINAL_FLAG64);
        } else {
          find_import_funcs(
              img.Read<PIMAGE_THUNK_DATA32>(descr->ImportNameTableRVA),
              IMAGE_ORDINAL_FLAG32);
        }
        imports.insert(import);
      }
      descr++;
    }
  }
  return imports;
}

PeImage::PeImage(const std::string& name, bool delayed)
    : Image(utils::lower(name)), delayed_(delayed) {}

const PIMAGE_FILE_HEADER LoadedImage::FileHeader() const {
  return &nt_headers_.x32->FileHeader;
}

const PIMAGE_DATA_DIRECTORY LoadedImage::DataDirectory() const {
  if (IsPe64()) return nt_headers_.x64->OptionalHeader.DataDirectory;
  return nt_headers_.x32->OptionalHeader.DataDirectory;
}

LoadedImage::~LoadedImage() {
  if (image_handle_) {
    ::FreeLibrary(image_handle_);
    image_handle_ = nullptr;
  }
}

const bool LoadedImage::IsPe64() const {
  return nt_headers_.x32->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64;
}

PeFunction::PeFunction(const std::string& name,
                       std::shared_ptr<PeImport> import)
    : name_(name), import_(import) {}

std::string PeFunction::String() const {
  if (!import_.expired()) {
    return import_.lock()->Name() + "!" + name_;
  }
  return name_;
}

const std::string& PeFunction::Name() const { return name_; }

PeImageFactory::PeImageFactory(bool delayed) : delayed_(delayed) {}

std::shared_ptr<Image> PeImageFactory::Create(const std::string& image) {
  auto image_ctx = std::make_shared<PeImage>(image, delayed_);
  image_ctx->Parse();
  return image_ctx;
}

PeMeta* PeMeta::instance_ = nullptr;

PeMeta& PeMeta::Instance() {
  if (!instance_) {
    instance_ = new PeMeta;
  }
  return *instance_;
}

PeMeta::PeMeta() {
  namespace_array_ = GetApiSetHeader();
  dll_name_re = std::wregex(L"(\\S+)(-l\\d+-\\d+-\\d+)", std::regex::optimize);
}

PeMeta::~PeMeta() {}

std::string PeMeta::VirtualToLogic(const std::string& virtual_dll) {
  if (namespace_array_ == NULL || (!utils::startswith(virtual_dll, "api-") &&
                                   !utils::startswith(virtual_dll, "ext-"))) {
    return virtual_dll;
  }

  auto logic_cache_it = logic_dll_cache_.find(virtual_dll);
  if (logic_cache_it != logic_dll_cache_.end()) {
    return logic_cache_it->second;
  }

  API_SET_NAMESPACE_ENTRY* ns_entry = namespace_array_->Entries;

  auto logic_dll = utils::a2w(virtual_dll);
  auto ext_start = logic_dll.find(L'.');
  if (ext_start != std::string::npos) {
    logic_dll = logic_dll.substr(0, ext_start);
  }
  logic_dll = VersionlessDllName(logic_dll);

  for (uint32_t i = 0; i < namespace_array_->Count; ++i, ++ns_entry) {
    if (ns_entry->NameLength) {
      std::wstring top_module(ns_entry->NameLength, L'\0');
      const auto top_ptr = ReadNamespace<const wchar_t*>(ns_entry->NameOffset);
      memcpy(top_module.data(), top_ptr, ns_entry->NameLength);
      top_module = VersionlessDllName(std::wstring(top_module.c_str()));
      if (top_module == logic_dll) {
        auto hosts = ReadNamespace<API_SET_VALUE_ENTRY*>(ns_entry->DataOffset);
        // Search from the end
        for (int j = ns_entry->HostsCount - 1; j >= 0; j--) {
          auto host = hosts[j];
          auto name_len = host.HostModuleNameLength / sizeof(wchar_t);
          if (name_len) {
            const auto name_ptr =
                ReadNamespace<const void*>(host.HostModuleName);
            logic_dll.resize(name_len, L'\0');
            auto dst =
                static_cast<void*>(const_cast<wchar_t*>(logic_dll.data()));
            memcpy(dst, name_ptr, name_len * sizeof(wchar_t));
            auto logic_ansii = utils::w2a(logic_dll);
            logic_dll_cache_[virtual_dll] = logic_ansii;
            return logic_ansii;
          }
        }
      }
    }
  }

  logic_dll_cache_[virtual_dll] = virtual_dll;
  return virtual_dll;
}

std::wstring PeMeta::VersionlessDllName(const std::wstring& name) {
  std::wsmatch groups;
  if (std::regex_match(name, groups, dll_name_re) && groups.size() > 1) {
    return groups[1].str();
  }
  return name;
}

PeImport::PeImport(const std::string& name, const std::string& alias)
    : Import(utils::lower(name), alias) {}
}  // namespace windep::image::pe
