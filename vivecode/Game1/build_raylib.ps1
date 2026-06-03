$ErrorActionPreference = "Stop"

$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

cmake -S . -B build-raylib -G Ninja `
    -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 `
    -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe

cmake --build build-raylib

Write-Host "Built build-raylib\ShadowWardRaylib.exe"

