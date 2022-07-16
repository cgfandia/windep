# windep

![ci](https://github.com/cgfandia/windep/actions/workflows/ci.yml/badge.svg)
[![codecov](https://codecov.io/gh/cgfandia/windep/branch/main/graph/badge.svg?token=HMS0OO7NRC)](https://codecov.io/gh/cgfandia-tii/DependencyWalker)

Small utility to find all DLL dependencies of the Windows PE binary. It can be helpful in terms of security and debugging.

## Usage

```
windep.exe [OPTION...] <root image>

-f, --functions Enable functions output
-d, --delayed Enable resolving of delayed imports
-h, --help Print help
-v, --version Print version
```
