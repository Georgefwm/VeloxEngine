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
glslc -fshader-stage=vertex   textured_quad.vert.glsl -o %shader_output_dir%\textured_quad.vert.spv
glslc -fshader-stage=fragment textured_quad.frag.glsl -o %shader_output_dir%\textured_quad.frag.spv

glslc -fshader-stage=vertex   vertex_base.vert.glsl   -o %shader_output_dir%\vertex_base.vert.spv
glslc -fshader-stage=fragment fragment_base.frag.glsl -o %shader_output_dir%\fragment_base.frag.spv
echo Done

