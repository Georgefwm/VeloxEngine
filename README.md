# Velox Engine

Simple 2D game engine.

## Build from source (windows)

#### Prerequisites
- LLVM (Clang & Clang++)
- VCPKG
- Ninja
- CMake

```powershell
git clone https://github.com/Georgefwm/VeloxEngine
```
```powershell
cd VeloxEngine
```\

Only need to run this once
```powershell
vcpkg install
```\

```powershell
./build
```\

```powershell
./run
```\

For rebuilds (after calling ./build)
```powershell
./fastbuild
```\

If you know what your doing, you can build using your own script based on the ones here.\

To run tests:
```powershell
./build_tests
```
```powershell
./test
```
