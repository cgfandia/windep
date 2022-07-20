# windep

![ci](https://github.com/cgfandia/windep/actions/workflows/ci.yml/badge.svg)
[![codecov](https://codecov.io/gh/cgfandia/windep/branch/main/graph/badge.svg?token=HMS0OO7NRC)](https://codecov.io/gh/cgfandia/windep)

Small utility to find all DLL dependencies of the Windows PE binary. It can be helpful in terms of security and debugging.

## Examples

### DOT

```shell
windep -f dot windep.exe
```

![graphviz](/media/graphviz.svg)

### JSON

```shell
windep -f json -c kernel32.dll
```

```json
{
  "kernel32.dll": {
    "imports": {
      "kernel32.dll": {
        "imports": {
          "kernelbase.dll": {
            "alias": "KERNELBASE.dll",
            "functions": ["AppContainerFreeMemory", "..."]
          },
          "ntdll.dll": {
            "alias": "api-ms-win-core-rtlsupport-l1-1-0.dll",
            "functions": ["RtlAddFunctionTable", "..."]
          }
        }
      },
      "kernelbase.dll": {
        "imports": {
          "kernelbase.dll": {
            "alias": "api-ms-win-eventing-provider-l1-1-0.dll",
            "functions": ["EventActivityIdControl", "..."]
          },
          "ntdll.dll": {
            "alias": "ntdll.dll",
            "functions": ["CsrAllocateCaptureBuffer", "..."]
          }
        }
      },
      "ntdll.dll": {
        "imports": {}
      }
    }
  }
}
```

## Usage

```
windep.exe [OPTION...] <binary>

  -c, --functions   Enable functions output
  -d, --delayed     Enable delayed imports
  -f, --format arg  Output format. Possible values: ascii, json, dot (default:
                    ascii)
  -t, --indent arg  Output rows indent (default: 2)
  -h, --help        Print help
  -v, --version     Print version
```
