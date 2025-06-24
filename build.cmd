@echo off

set buildPresetName=Debug

echo Generating build...
cmake --preset=%buildPresetName% -GNinja -Wno-deprecated
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

echo Building...
cmake --build --preset app-%buildPresetName%
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done

echo Copying assets to output dir...
mkdir "build\bin\%buildPresetName%\assets" >nul 2>nul
xcopy "assets\*" "build\bin\%buildPresetName%\assets" /E /H /C /I /Y >nul 2>nul
echo done

echo Copying shaders to output dir...
mkdir "build\bin\%buildPresetName%\shaders" >nul 2>nul
xcopy "Velox\shaders\*" "build\bin\%buildPresetName%\shaders" /E /H /C /I /Y >nul 2>nul
echo done


