mkdir "build"
cd "build"

cmake -G Ninja ..
if %errorlevel% neq 0 exit /b %errorlevel%

ninja
if %errorlevel% neq 0 exit /b %errorlevel%

:: Now compile shaders and place them in the bin output
cd "bin"
mkdir "shaders"
cd "shaders"

glslc -fshader-stage=vertex ..\..\..\Velox\shaders\vertex_base.glsl -o vertex_base.spv
glslc -fshader-stage=fragment ..\..\..\Velox\shaders\fragment_base.glsl -o fragment_base.spv

