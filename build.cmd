@echo off

mkdir "build" >nul 2>nul
cd "build"

echo Generating ninja build...
cmake -G Ninja -DBUILD_VELOX_TESTS=OFF -Wno-deprecated ..
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

echo Building...
ninja
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

:: Back to root dir
cd ".."

echo Copying assets to output dir...
mkdir "build\bin\assets" >nul 2>nul
xcopy "assets\*" "build\bin\assets" /E /H /C /I /Y >nul 2>nul
echo done

./compile_shaders.cmd

