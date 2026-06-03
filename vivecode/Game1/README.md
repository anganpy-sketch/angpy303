# Shadow Ward

An original lightweight C++ survival-horror prototype inspired by fixed-camera classics.

This is not a Resident Evil clone using Capcom IP. It keeps the genre feel: fixed security-camera angles, tank controls, scarce ammo, keys, slow enemies, and oppressive low-poly rooms.

## Best Build: raylib 3D

Requirements:

- Windows
- MSYS2 UCRT64 packages: `raylib`, `cmake`, and `ninja`

From this folder:

```powershell
.\build_raylib.bat
```

Run:

```powershell
.\build-raylib\ShadowWardRaylib.exe
```

## Fallback Build: tiny software renderer

Requirements:

- Windows
- `g++` available on PATH, such as MSYS2 UCRT64

From this folder:

```powershell
.\build.bat
```

If your PowerShell policy allows scripts, this also works:

```powershell
.\build.ps1
```

Run:

```powershell
.\build\ShadowWard.exe
```

## Controls

- `W` / `S`: move forward/back
- `A` / `D`: turn
- `Space`: fire
- `E`: interact / pick up
- `R`: restart after win or death
- `Esc`: quit

## What is inside

- Tiny software 3D renderer using Win32 + GDI only
- raylib-powered lightweight 3D edition with procedural low-poly 3D assets
- Fixed-camera rooms
- Tank controls
- Collision
- Procedural lighting/fog
- OBJ mesh assets and loader kept in the fallback renderer
- Enemy AI
- Ammo, health, key, locked exit

## Crash note

The raylib build originally used OBJ models directly. On this Windows/MSYS2 setup that path caused a `0xc0000005` access violation after a few seconds. The current raylib edition avoids that unstable loader path and draws the 3D actors/items procedurally.
