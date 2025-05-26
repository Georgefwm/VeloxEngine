@echo off

mkdir "build" >nul 2>nul
cd "build"

echo Generating ninja build...
cmake -G Ninja -DBUILD_VELOX_TESTS=OFF ..
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

echo Building...
ninja
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

cd "bin"
mkdir "shaders" >nul 2>nul

:: Now compile shaders and place them in the bin output
cd "..\..\Velox\shaders"

set shader_output_dir="..\..\build\bin\shaders"

echo Compiling shaders...
glslc -fshader-stage=vertex   vertex_base.glsl   -o %shader_output_dir%\vertex_base.spv
glslc -fshader-stage=fragment fragment_base.glsl -o %shader_output_dir%\fragment_base.spv
echo Done

