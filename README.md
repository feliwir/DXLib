# DXLIB

## DirectDraw (current status)

This repository currently implements the `ddraw` compatibility layer only.

## Build

The project exposes two DirectDraw targets:

- `dxlib::ddraw`: static link target for regular CMake integration.
- `ddraw` (alias `dxlib::ddraw_shim`): shared shim target for drop-in deployment.

Example build:

```bash
cmake -S . -B build
cmake --build build
```

## Usage modes

1. Shim deployment (Windows-style):
Place the built shared library next to your game/application executable (for example `ddraw.dll` on Windows).

2. Regular CMake linking:
Link against `dxlib::ddraw` from your own CMake project, which is especially useful on Linux.

## Platform-specific window handling

- On non-Windows platforms, `HWND` is treated as an `SDL_Window*` handle.
- On Windows, `SetCooperativeLevel` uses `SDL_CreateWindowWithProperties` to create an SDL window from the provided native `HWND`.


