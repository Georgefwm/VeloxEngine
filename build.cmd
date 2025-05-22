mkdir "build"
cd "build"

cmake -G Ninja ..
if %errorlevel% neq 0 exit /b %errorlevel%

ninja
if %errorlevel% neq 0 exit /b %errorlevel%

cd "bin"
mkdir "shaders"

:: Now compile shaders and place them in the bin output
cd "..\..\Velox\shaders"

glslc -fshader-stage=vertex vertex_base.glsl -o ..\..\build\bin\shaders\vertex_base.spv
glslc -fshader-stage=fragment fragment_base.glsl -o ..\..\build\bin\shaders\fragment_base.spv


