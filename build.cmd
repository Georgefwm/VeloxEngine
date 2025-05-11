:: Compile shaders
:: glslc (glsl_shader_file_name) -o (output_shader_name)
:: if %errorlevel% neq 0 exit /b 1
:: exit on fail

mkdir "build"
cd "build"
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G Ninja ..
:: exit on fail

ninja
cd ..
:: exit on fail
