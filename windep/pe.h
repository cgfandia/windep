// copyright MIT License Copyright (c) 2021, Albert Farrakhov

#pragma once
#include <Windows.h>
#include <winternl.h>

#include <memory>
#include <regex>
#include <string>
#include <unordered_map>

#include "exceptions.h"
#include "image.h"

namespace windep::image::pe {
class PeImport : public Import {
 public:
  PeImport(const std::string& name, const std::string& alias);
};

class PeFunction : public Function {
  std::string name_;
  std::weak_ptr<PeImport> import_;

 public:
  explicit PeFunction(const std::string& name,
                      std::shared_ptr<PeImport> import);
  std::string String() const override;
};

class LoadedImage {
  HMODULE image_handle_ = nullptr;
  LPVOID image_view_ = nullptr;
  PIMAGE_DOS_HEADER dos_header_ = nullptr;
  union {
    PIMAGE_NT_HEADERS32 x32 = nullptr;
    PIMAGE_NT_HEADERS64 x64;
  } nt_headers_;
  PIMAGE_SECTION_HEADER section_headers_ = nullptr;
  PIMAGE_SECTION_HEADER last_section_ = nullptr;

 public:
  explicit LoadedImage(const std::string& name);
  virtual ~LoadedImage();
  LoadedImage(const LoadedImage&) = delete;
  LoadedImage(LoadedImage&&) = delete;
  LoadedImage& operator=(const LoadedImage&) = delete;
  LoadedImage& operator=(LoadedImage&&) = delete;
  const bool IsPe64() const;
  template <typename T>
  T MapOffset(ULONGLONG offset) const {
    return reinterpret_cast<T>(static_cast<PBYTE>(image_view_) + offset);
  }
  const PIMAGE_FILE_HEADER FileHeader() const;
  const PIMAGE_DATA_DIRECTORY DataDirectory() const;
};

class PeImage : public Image {
  bool delayed_;
  void ParseImports(const LoadedImage& loaded_image);
  void ParseDelayedImports(const LoadedImage& loaded_image);

 public:
  PeImage(const std::string& name, bool delayed);
  void Parse() override;
};

class PeImageFactory : public ImageContextFactory {
  bool delayed_ = false;

 public:
  explicit PeImageFactory(bool delayed = false);
  std::shared_ptr<Image> Create(const std::string& image) override;
};

#define MKPTR(p1, p2) ((DWORD_PTR)(p1) + (DWORD_PTR)(p2))

typedef struct _stripped_peb32 {
  BYTE unused1[0x038];
  PVOID ApiSet;
  BYTE unused2[0x1AC];
} PEB32;

typedef struct _stripped_peb64 {
  BYTE unused1[0x068];
  PVOID ApiSet;
  BYTE unused2[0x23C];
} PEB64;

typedef struct _PROCESS_BASIC_INFORMATION {
  PVOID Reserved1;
  LPVOID PebBaseAddress;
  PVOID Reserved2[2];
  ULONG_PTR UniqueProcessId;
  PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

typedef struct _api_set_value_entry {
  DWORD Flags;
  DWORD ImportModuleName;
  WORD ImportModuleNameLength;
  DWORD HostModuleName;
  WORD HostModuleNameLength;
} API_SET_VALUE_ENTRY;

typedef struct _api_set_value_array {
  DWORD Flags;
  DWORD NumberOfHosts;
  API_SET_VALUE_ENTRY Array[1];
} API_SET_VALUE_ARRAY;

typedef struct _api_set_namespace_entry {
  DWORD Flags;
  DWORD NameOffset;
  DWORD NameLength;
  DWORD NameHashSize;
  DWORD DataOffset;
  DWORD HostsCount;
} API_SET_NAMESPACE_ENTRY;

typedef struct _api_set_namespace_array {
  DWORD Version;
  DWORD Size;
  DWORD Flags;
  DWORD Count;
  DWORD NamespaceEntriesOffset;
  DWORD HashEntriesOffset;
  DWORD HashMultiplier;
  API_SET_NAMESPACE_ENTRY Entries[1];
} API_SET_NAMESPACE_ARRAY;

using pfnNtQueryInformationProcess =
    NTSTATUS(NTAPI*)(HANDLE ProcessHandle, DWORD ProcessInformationClass,
                     PVOID ProcessInformation, ULONG ProcessInformationLength,
                     PULONG ReturnLength);

class PeMeta {
  static PeMeta* instance_;
  API_SET_NAMESPACE_ARRAY* namespace_array_;
  std::unordered_map<std::string, std::string> logic_dll_cache_;
  std::wregex dll_name_re;

  PeMeta();
  ~PeMeta();
  PeMeta(const PeMeta&) = delete;
  PeMeta& operator=(const PeMeta&) = delete;
  PeMeta(PeMeta&&) = delete;
  PeMeta& operator=(PeMeta&&) = delete;
  template <typename T>
  T MapNamespaces(uintptr_t offset) {
    return reinterpret_cast<T>(reinterpret_cast<PBYTE>(namespace_array_) +
                               offset);
  }

 public:
  static PeMeta& Instance();
  std::string VirtualToLogic(const std::string& virtual_dll);
  std::wstring VersionlessDllName(const std::wstring& name);
};
}  // namespace windep::image::pe
