@echo off
setlocal

if not exist build mkdir build

g++ -std=c++20 -O2 -Wall -Wextra -municode ^
    src\main.cpp ^
    -o build\ShadowWard.exe ^
    -lgdi32 -luser32

if errorlevel 1 exit /b %errorlevel%
echo Built build\ShadowWard.exe

