@echo off

mkdir "build" > $null 2>&1
cd "build"

echo Generating ninja build...
cmake -G Ninja -DBUILD_VELOX_TESTS=ON ..
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

echo Building...
ninja
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

cd "bin"
mkdir "shaders" > $null 2>&1

:: Now compile shaders and place them in the bin output
cd "..\..\Velox\shaders"

echo Compiling shaders...
glslc -fshader-stage=vertex vertex_base.glsl -o ..\..\build\bin\shaders\vertex_base.spv
glslc -fshader-stage=fragment fragment_base.glsl -o ..\..\build\bin\shaders\fragment_base.spv
echo Done

