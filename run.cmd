@echo off

.\build\bin\App.exe
if %errorlevel% neq 0 echo App not built, please run build.cmd
