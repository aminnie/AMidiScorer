# MidiScorer

MidiScorer is a JUCE/C++ standalone desktop app that reads MIDI files, renders a selected track as score-like notation, detects chords, and follows playback with a rolling 5-bar view.

## Current v1 capabilities

- Load `.mid` / `.midi` files.
- Select one MIDI track for analysis and display.
- Build tempo and time-signature maps from MIDI meta events.
- Quantize note starts and durations to:
  - 1/16, 1/8, 1/4, 1/2, whole
- Render a score-style staff view with:
  - bar headers and beat guides
  - noteheads, stems, flags, tie hinting
  - basic accidental and ledger-line cues
- Detect and display chords above the score:
  - triads, sevenths, extensions/alterations, slash bass forms
- Chord naming options:
  - sharp vs flat root names
  - plain vs jazz alias text
- Chord source options:
  - selected track only
  - first 5 tracks (default)
- Playback-synced rolling window:
  - previous 2 bars + current bar + next 2 bars

## Project structure

- `CMakeLists.txt` - JUCE/CMake project setup
- `Main.cpp` - JUCE application entry point
- `src/app/MainComponent.h` - UI controls, file load flow, orchestration
- `src/midi/TempoMap.h` - tempo/time-signature and bar mapping
- `src/midi/TrackNoteExtractor.h` - note-on/note-off pairing
- `src/midi/MidiProjectLoader.h` - MIDI ingestion into app model
- `src/notation/Quantizer.h` - rhythmic quantization to v1 note values
- `src/notation/ScoreModel.h` - score/bar/chord model
- `src/notation/ScoreRenderer.h` - score drawing
- `src/harmony/ChordDetector.h` - chord analysis and naming options
- `src/playback/PlaybackClock.h` - playback timing
- `src/playback/PlaybackController.h` - playback state + current bar
- `tests/test_main.cpp` - lightweight core tests
- `tests/fixtures/` - fixture specs/documentation

## Requirements

- CMake 3.22+
- C++17 compiler
- JUCE source checkout available in one of:
  - `-DJUCE_ROOT=<path-to-JUCE>`
  - `.deps/JUCE` under this project
  - `C:/JUCE` (Windows auto-detected)

## Build (Windows example)

```powershell
cmake -S . -B build -DJUCE_ROOT="C:/JUCE"
cmake --build build --config Debug --target MidiScorer
```

Output executable:

- `build/MidiScorer_artefacts/Debug/MidiScorer.exe`

## Run tests

```powershell
cmake --build build --config Debug --target MidiScorerTests
ctest --test-dir build -C Debug --output-on-failure
```

## How to use

1. Launch `MidiScorer`.
2. Click **Load MIDI** and choose a MIDI file.
3. Select a track from the **Track** dropdown.
4. Optionally choose chord naming preferences:
   - accidental style (sharp/flat)
   - alias style (plain/jazz)
5. Choose chord analysis source:
   - selected track
   - first 5 tracks
6. Click **Play** to follow the rolling 5-bar score.
7. Click **Stop** to stop playback.

## Notes and known limitations (v1)

- Rendering is intentionally simplified (single-voice style).
- Quantization is restricted to 1/16 through whole note values.
- Rests, beaming, and accidentals are heuristic, not full engraving rules.
- Chord detection uses deterministic template scoring and can be ambiguous for dense voicings.
- Playback currently drives visual sync; MIDI audio output routing is not implemented as a full DAW transport.

## Roadmap ideas

- Improved engraving rules (meter-aware beaming, richer rest logic, key-aware accidentals)
- More advanced chord preference/profile settings
- Optional piano-roll overlay
- Export score snapshots or MusicXML bridge

## Developer Notes

- `MIDI ingest pipeline`
  - Entry point is `MidiProjectLoader::load()` in `src/midi/MidiProjectLoader.h`.
  - This is where file parse, meta extraction, per-track note extraction, and project-level duration/bar bounds are assembled.
  - Add new metadata extraction here first (markers, lyrics, key changes, etc.).

- `Tempo/bar math`
  - `src/midi/TempoMap.h` is the authoritative timing layer.
  - Extend this module if you need alternate transport domains (ticks-only, SMPTE, swing grids, custom bar math).
  - Keep new conversions centralized here to avoid timing drift across renderer/chord logic/playback.

- `Quantization extension points`
  - `Quantizer::quantizeTrack()` in `src/notation/Quantizer.h` controls note onset/duration normalization.
  - For tuplets or dotted values, add to `quantizeDuration()` + `quarterToValue()` and then update renderer symbol mapping.
  - If you need style presets (strict vs humanized), add strategy options to quantizer input.

- `Chord rules and naming`
  - Harmonic matching lives in `ChordDetector` (`src/harmony/ChordDetector.h`).
  - Add/adjust quality templates in `templates()`.
  - Naming behavior is controlled via `NamingOptions` (accidental preference + alias style); this is the right place for custom user profiles.

- `Notation rendering`
  - `src/notation/ScoreRenderer.h` contains all drawing logic (staff, noteheads, stems, ties, beat guides, accidental cues, beam hints).
  - Keep visual-only decisions here; do not move timing/harmony rules into renderer.
  - For richer engraving, introduce a pre-render layout pass (collision avoidance, stem direction groups, rest placement) before direct draw calls.

- `Playback sync and rolling window`
  - `PlaybackClock` and `PlaybackController` (`src/playback/`) drive elapsed time and current bar.
  - `MainComponent::timerCallback()` updates transport text and the rolling 5-bar center.
  - If adding audio/MIDI output transport later, keep this controller as the single source of visual playhead truth.

- `UI wiring and state orchestration`
  - `src/app/MainComponent.h` coordinates file load, track selection, chord naming preferences, score rebuild, and transport controls.
  - Any new user-facing preference should be wired to trigger `rebuildFromSelectedTrack()` or a dedicated incremental refresh path.

- `Testing guidance`
  - Core logic tests are in `tests/test_main.cpp`.
  - Add regression tests for new tempo/time-signature edge cases and chord-template ambiguities first, then add UI-facing checks.
  - Keep fixture definitions in `tests/fixtures/fixture-specs.md` deterministic and minimal.

## First contribution checklist

1. Configure and build locally:
   - `cmake -S . -B build -DJUCE_ROOT="C:/JUCE"`
   - `cmake --build build --config Debug --target MidiScorer MidiScorerTests`
2. Run tests:
   - `ctest --test-dir build -C Debug --output-on-failure`
3. Launch app and smoke test:
   - load a MIDI file
   - switch tracks
   - toggle chord naming preferences
   - run Play/Stop and confirm rolling bar movement
4. Read key files in order:
   - `src/app/MainComponent.h`
   - `src/midi/MidiProjectLoader.h`
   - `src/midi/TempoMap.h`
   - `src/notation/Quantizer.h`
   - `src/notation/ScoreModel.h`
   - `src/notation/ScoreRenderer.h`
   - `src/harmony/ChordDetector.h`
5. Pick one safe extension path:
   - add a new chord template
   - improve one quantizer rule
   - add one renderer refinement with test coverage
6. Keep refactors bounded:
   - avoid mixing timing-model changes with drawing changes in the same PR
   - update tests when touching `TempoMap`, `Quantizer`, or `ChordDetector`
   - preserve `PlaybackController` as the source of playhead/bar truth
