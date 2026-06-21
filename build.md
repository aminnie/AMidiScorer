# MidiScorer build guide

This document describes how to configure, build, test, and run MidiScorer locally.

**Windows** instructions are current and tested. **macOS** support is planned; a section is reserved below for future notes.

## Requirements

- **CMake** 3.22 or newer
- **C++17** compiler
  - Windows: Visual Studio 2022 (MSVC) with the Desktop development with C++ workload
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

## macOS

macOS build steps are not documented here yet. When added, this section will cover:

- Xcode / command-line tools prerequisites
- JUCE path conventions on macOS
- Configure, build, test, and `.app` bundle launch paths

Until then, use the same CMake targets (`MidiScorer`, `MidiScorerTests`) with a native generator (for example `-G Xcode` or Ninja) and a local JUCE checkout via `-DJUCE_ROOT`.

## Quick reference

| Task | Windows (Debug) |
|------|-----------------|
| Configure | `cmake -S . -B build -DJUCE_ROOT="C:/JUCE"` |
| Build app | `cmake --build build --config Debug --target MidiScorer` |
| Build + test | `cmake --build build --config Debug --target MidiScorer MidiScorerTests` |
| Run tests | `ctest --test-dir build -C Debug --output-on-failure` |
| Launch | `Start-Process "build/MidiScorer_artefacts/Debug/MidiScorer.exe"` |

## Related docs

- `README.md` — features and project overview
- `CONTRIBUTING.md` — contributor workflow and smoke-test checklist
- `AGENT.md` — agent-oriented build/test commands
