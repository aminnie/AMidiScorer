# MidiScorer build guide

This document describes how to configure, build, test, and run MidiScorer locally on Windows and macOS.

## Requirements

- **CMake** 3.22 or newer
- **C++17** compiler
  - Windows: Visual Studio 2022 (MSVC) with the Desktop development with C++ workload
  - macOS: Xcode or Command Line Tools (`xcode-select --install`)
- **JUCE** source checkout (see [JUCE location](#juce-location))

## JUCE location

CMake resolves JUCE in this order:

1. `-DJUCE_ROOT=<path-to-JUCE>` on the configure command line
2. `.deps/JUCE` inside this repository
3. `C:/JUCE` on Windows (auto-detected)
4. `$JUCE_ROOT` environment variable

If none of these exist, configuration fails with a message to clone JUCE or set `JUCE_ROOT`.

Example clone into the project:

```powershell
git clone https://github.com/juce-framework/JUCE.git .deps/JUCE
```

## Windows

All commands below assume the repository root (`midiscorer/`) as the current directory.

### First-time configure

```powershell
cmake -S . -B build -DJUCE_ROOT="C:/JUCE"
```

Use your actual JUCE path if it differs. Omit `-DJUCE_ROOT` when using `.deps/JUCE` or `C:/JUCE`.

CMake generates a Visual Studio solution under `build/`. The default configuration used in these notes is **Debug**.

### Build the app

```powershell
cmake --build build --config Debug --target MidiScorer
```

Build the app and unit tests together:

```powershell
cmake --build build --config Debug --target MidiScorer MidiScorerTests
```

### Run tests

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

### Preferred Windows helper script

```powershell
.\scripts\build.ps1 -Configuration Debug -Target All
```

This script configures, builds app+tests, and runs `ctest`.

### Launch the app

Debug executable:

```powershell
Start-Process "build/MidiScorer_artefacts/Debug/MidiScorer.exe"
```

Or run directly:

```powershell
.\build\MidiScorer_artefacts\Debug\MidiScorer.exe
```

### Release build

Configure once (same as above), then:

```powershell
cmake --build build --config Release --target MidiScorer
```

Release executable:

- `build/MidiScorer_artefacts/Release/MidiScorer.exe`

### Rebuild tips (Windows)

- If the app is already running, close it before rebuilding, or stop the process:

  ```powershell
  taskkill /IM MidiScorer.exe /F
  ```

- After pulling changes that touch `CMakeLists.txt` or JUCE modules, re-run configure:

  ```powershell
  cmake -S . -B build -DJUCE_ROOT="C:/JUCE"
  ```

- To do a clean rebuild, delete the `build/` folder and configure again.

### Windows output paths

| Target | Debug | Release |
|--------|-------|---------|
| App | `build/MidiScorer_artefacts/Debug/MidiScorer.exe` | `build/MidiScorer_artefacts/Release/MidiScorer.exe` |
| Tests | `build/MidiScorerTests_artefacts/Debug/MidiScorerTests.exe` | `build/MidiScorerTests_artefacts/Release/MidiScorerTests.exe` |

## Application icon

MidiScorer follows the same pattern as AMidiOrgan:

- Source artwork: `src/resources/icons/app-icon-master.png` (1024×1024 PNG with alpha)
- **Windows:** CMake `ICON_BIG` / `ICON_SMALL` embed the PNG into the `.exe` (taskbar, title bar, Alt+Tab)
- **macOS:** run `scripts/build-macos-icon.sh` on a Mac to produce `src/resources/icons/MidiScorer.icns`, then rebuild the app bundle

After changing the icon PNG, re-run configure and rebuild so JUCE regenerates the embedded Windows icon resources. To tweak artwork size/padding, run `scripts/regenerate_app_icon.ps1` and rebuild.

## macOS

All commands below assume the repository root (`midiscorer/`) as the current directory.

### Summary

```bash
git clone ...JUCE... .deps/JUCE
./scripts/mac-build.sh
open "build-mac/MidiScorer_artefacts/Debug/MidiScorer.app"
```

`./scripts/mac-build.sh` configures, builds `MidiScorer` + `MidiScorerTests`, and runs `ctest`.

### First-time configure/build/test manually (Unix Makefiles)

```bash
cmake -S . -B build-mac -DJUCE_ROOT="$PWD/.deps/JUCE" -DCMAKE_BUILD_TYPE=Debug
```

Use your actual JUCE path if needed. You can also set `JUCE_ROOT` in your environment.

### Build app and tests

```bash
cmake --build build-mac --target MidiScorer MidiScorerTests
```

### Run tests

```bash
ctest --test-dir build-mac --output-on-failure
```

### Launch the app

```bash
open "build-mac/MidiScorer_artefacts/Debug/MidiScorer.app"
```

### Alternate generators

- Ninja:
  ```bash
  cmake -S . -B build-mac -G Ninja -DJUCE_ROOT="$PWD/.deps/JUCE" -DCMAKE_BUILD_TYPE=Debug
  cmake --build build-mac --target MidiScorer MidiScorerTests
  ```
- Xcode:
  ```bash
  cmake -S . -B build-mac-xcode -G Xcode -DJUCE_ROOT="$PWD/.deps/JUCE"
  cmake --build build-mac-xcode --config Debug --target MidiScorer MidiScorerTests
  ctest --test-dir build-mac-xcode -C Debug --output-on-failure
  open "build-mac-xcode/MidiScorer_artefacts/Debug/MidiScorer.app"
  ```

### macOS helper scripts

- `./scripts/mac-build.sh` supports `--release`, `--ninja`, and `--xcode` and runs tests by default.
- `./scripts/mac-bootstrap.sh` is a first-time setup helper that verifies tools, clones JUCE if missing, then runs `mac-build.sh`.

### macOS output paths

| Target | Debug (Makefile/Ninja) | Release (Makefile/Ninja) |
|--------|--------------------------|---------------------------|
| App | `build-mac/MidiScorer_artefacts/Debug/MidiScorer.app` | `build-mac-release/MidiScorer_artefacts/Release/MidiScorer.app` |
| Tests | `build-mac/MidiScorerTests_artefacts/Debug/MidiScorerTests` | `build-mac-release/MidiScorerTests_artefacts/Release/MidiScorerTests` |

## Quick reference

| Task | Windows (Debug) |
|------|-----------------|
| Configure | `cmake -S . -B build -DJUCE_ROOT="C:/JUCE"` |
| Build app | `cmake --build build --config Debug --target MidiScorer` |
| Build + test | `cmake --build build --config Debug --target MidiScorer MidiScorerTests` |
| Run tests | `ctest --test-dir build -C Debug --output-on-failure` |
| Launch | `Start-Process "build/MidiScorer_artefacts/Debug/MidiScorer.exe"` |
| Script helper | `.\scripts\build.ps1 -Configuration Debug -Target All` |

| Task | macOS (Debug, Makefile/Ninja) |
|------|--------------------------------|
| Configure | `cmake -S . -B build-mac -DJUCE_ROOT="$PWD/.deps/JUCE" -DCMAKE_BUILD_TYPE=Debug` |
| Build app | `cmake --build build-mac --target MidiScorer` |
| Build + test | `cmake --build build-mac --target MidiScorer MidiScorerTests` |
| Run tests | `ctest --test-dir build-mac --output-on-failure` |
| Launch | `open "build-mac/MidiScorer_artefacts/Debug/MidiScorer.app"` |

## Related docs

- `README.md` — features and project overview
- `CONTRIBUTING.md` — contributor workflow and smoke-test checklist
- `AGENT.md` — agent-oriented build/test commands
