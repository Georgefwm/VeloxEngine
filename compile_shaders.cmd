@echo off
setlocal enableextensions

echo Compiling shaders...

set shader_source_dir="Velox\shaders"

mkdir "build\bin\shaders" >nul 2>nul
set shader_output_dir="build\bin\shaders"

glslc -fshader-stage=vertex   %shader_source_dir%\indexed_textures.vert.glsl -o %shader_output_dir%\indexed_textures.vert.spv
glslc -fshader-stage=fragment %shader_source_dir%\indexed_textures.frag.glsl -o %shader_output_dir%\indexed_textures.frag.spv
glslc -fshader-stage=vertex   %shader_source_dir%\textured_quad.vert.glsl -o %shader_output_dir%\textured_quad.vert.spv
glslc -fshader-stage=fragment %shader_source_dir%\textured_quad.frag.glsl -o %shader_output_dir%\textured_quad.frag.spv
glslc -fshader-stage=vertex   %shader_source_dir%\vertex_base.vert.glsl   -o %shader_output_dir%\vertex_base.vert.spv
glslc -fshader-stage=fragment %shader_source_dir%\fragment_base.frag.glsl -o %shader_output_dir%\fragment_base.frag.spv

echo Done

