@echo off

echo Generating ninja build...
cmake --preset=Debug -B build -Wno-deprecated
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

echo Building...
ninja -C build
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

echo Copying assets to output dir...
mkdir "build\bin\Debug\assets" >nul 2>nul
xcopy "assets\*" "build\bin\Debug\assets" /E /H /C /I /Y >nul 2>nul
echo done

echo Copying shaders to output dir...
mkdir "build\bin\Debug\shaders" >nul 2>nul
xcopy "Velox\shaders\*" "build\bin\Debug\shaders" /E /H /C /I /Y >nul 2>nul
echo done


