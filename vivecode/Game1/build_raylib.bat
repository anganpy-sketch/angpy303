@echo off
setlocal

set PATH=C:\msys64\ucrt64\bin;%PATH%

cmake -S . -B build-raylib -G Ninja ^
    -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 ^
    -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe

if errorlevel 1 exit /b %errorlevel%

cmake --build build-raylib

if errorlevel 1 exit /b %errorlevel%
echo Built build-raylib\ShadowWardRaylib.exe

