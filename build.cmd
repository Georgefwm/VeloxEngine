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
mkdir "build\%buildPresetName%\bin\assets" >nul 2>nul
xcopy "assets\*" "build\%buildPresetName%\bin\assets" /E /H /C /I /Y >nul 2>nul
echo done

echo Copying shaders to output dir...
mkdir "build\%buildPresetName%\bin\shaders" >nul 2>nul
xcopy "Velox\shaders\*" "build\%buildPresetName%\bin\shaders" /E /H /C /I /Y >nul 2>nul
echo done


