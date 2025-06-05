@echo off

mkdir "build" >nul 2>nul
cd "build"

echo Building...
ninja
if %errorlevel% neq 0 exit /b %errorlevel%
echo Done
