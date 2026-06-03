$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path "build" | Out-Null

g++ -std=c++20 -O2 -Wall -Wextra -municode `
    src\main.cpp `
    -o build\ShadowWard.exe `
    -lgdi32 -luser32

Write-Host "Built build\ShadowWard.exe"

