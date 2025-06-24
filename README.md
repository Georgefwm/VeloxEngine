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
```

Only need to run this once
```powershell
vcpkg install
```
```powershell
./build
```
```powershell
./run
```
For rebuilds (after calling ./build)
```powershell
./fastbuild
```

To run tests:
```powershell
./build_tests
```
```powershell
./test
```
