@echo off

.\build\bin\VeloxTests.exe > $null 2>&1
if %errorlevel% neq 0 echo Tests not built, please run build_tests.cmd
