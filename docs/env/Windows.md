# Windows development environment

Having `cmake` already available is a plus, otherwise `vcpkg` will fetch one.

```powershell
winget install Kitware.CMake
```

## `vcpkg` + `WinLibs`

### Install `vcpkg` in `%LOCALAPPDATA%`

For portability, `vcpkg` is used as dependencies manager. To install in Win, run these commands. Best to put it in a path without spaces, I chose `%LOCALAPPDATA%`.

```powershell
cd $env:LOCALAPPDATA
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

If a `winget` package for `vcpkg` ever becomes available and reliable, let me know.

### Install `WinLibs`

Extract [latest](https://winlibs.com/) `GCC (with POSIX threads) + LLVM/Clang/LLD/LLDB + MinGW-w64 (UCRT)` on a path without spaces, and add the bin folder at the top of `PATH`. Doing it with `winget` does not work because it gets placed in a path with spaces.

### Compile

Move into `engines/cpp` and to fetch and build dependencies run

```powershell
cmake --preset vcpkg-ninja
```

To build the executable run

```powershell
cmake --build --preset release
```

And to test it

```powershell
ctest --preset run-tests
```
