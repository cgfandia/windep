# windep

![ci](https://github.com/cgfandia/windep/actions/workflows/ci.yml/badge.svg)
[![codecov](https://codecov.io/gh/cgfandia/windep/branch/main/graph/badge.svg?token=HMS0OO7NRC)](https://codecov.io/gh/cgfandia/windep)

Small utility to find all DLL dependencies of the Windows PE binary. It can be helpful in terms of security and debugging.

## Features

- Delayed imports
- Virtual DLL resolving, [Runtime DLL name resolution: ApiSetSchema - Part I](https://blog.quarkslab.com/runtime-dll-name-resolution-apisetschema-part-i.html)
- Various output formats:
  - Tree or ASCII
  - DOT or Graphviz
  - JSON
  - CSV

## Examples

### Tree/ASCII

```shell
windep kernel32.dll
```

```
kernel32.dll
  kernelbase.dll
    kernelbase.dll
    ntdll.dll
  ntdll.dll
```

### DOT

```shell
windep -F dot windep.exe
```

![graphviz](/media/graphviz.svg)

### JSON

```shell
windep -f -F json kernel32.dll
```

```json
{
  "kernel32.dll": {
    "imports": {
      "kernel32.dll": {
        "imports": {
          "kernelbase.dll": {
            "alias": "KERNELBASE.dll",
            "functions": ["AppContainerFreeMemory", "..."],
            "unresolved": false
          },
          "ntdll.dll": {
            "alias": "api-ms-win-core-rtlsupport-l1-1-0.dll",
            "functions": ["RtlAddFunctionTable", "..."],
            "unresolved": false
          }
        },
        "path": "C:\\Windows\\System32\\KERNEL32.DLL"
      },
      "kernelbase.dll": {
        "imports": {
          "kernelbase.dll": {
            "alias": "api-ms-win-eventing-provider-l1-1-0.dll",
            "functions": ["EventActivityIdControl", "..."],
            "unresolved": false
          },
          "ntdll.dll": {
            "alias": "ntdll.dll",
            "functions": ["CsrAllocateCaptureBuffer", "..."],
            "unresolved": false
          }
        },
        "path": "C:\\Windows\\System32\\KERNELBASE.dll"
      },
      "ntdll.dll": {
        "imports": {},
        "path": "C:\\Windows\\SYSTEM32\\ntdll.dll"
      }
    }
  }
}
```

### CSV

```shell
windep -F csv kernel32.dll
```

```csv
Source,Target
kernel32.dll,kernelbase.dll
kernel32.dll,ntdll.dll
kernelbase.dll,kernelbase.dll
kernelbase.dll,ntdll.dll
```

## Usage

```
windep.exe [OPTION...] <binary>

  -f, --functions   Enable functions output
  -d, --delayed     Enable delayed imports
  -F, --format arg  Output format. Possible values: ascii, json, dot, csv (default: ascii)
  -I, --indent arg  Output rows indent (default: 2)
  -o, --output arg  File output (default: "")
  -h, --help        Print help
  -v, --version     Print version
```
