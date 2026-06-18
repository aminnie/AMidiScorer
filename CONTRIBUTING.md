# Contributing to MidiScorer

Thanks for contributing. This project is a JUCE/C++ app focused on MIDI parsing, score-style rendering, chord analysis, and playback-synced visualization.

## Quick start

1. Configure:
   - `cmake -S . -B build -DJUCE_ROOT="C:/JUCE"`
2. Build:
   - `cmake --build build --config Debug --target MidiScorer MidiScorerTests`
3. Test:
   - `ctest --test-dir build -C Debug --output-on-failure`

## Expected change scope

Keep pull requests small and focused. Preferred examples:

- one chord-detection rule change + tests
- one quantization improvement + tests
- one rendering refinement + manual smoke notes

Avoid mixing unrelated subsystems in one PR (for example, timing model + UI drawing + chord templates all at once).

## Core architecture guardrails

- `TempoMap` is the timing source of truth.
- `PlaybackController` is the playback/bar source of truth.
- `ScoreRenderer` should remain visual-only (no timing or harmonic logic decisions).
- `ChordDetector` should own chord matching and naming policy.

## Testing expectations

For changes in:

- `src/midi/TempoMap.h`: add/adjust deterministic timing tests.
- `src/notation/Quantizer.h`: add quantization behavior checks.
- `src/harmony/ChordDetector.h`: add chord match and naming-option checks.

At minimum before submitting:

1. Build app and tests successfully.
2. Run `ctest` with zero failures.
3. Smoke test in app:
   - load MIDI
   - switch track
   - verify chord naming preference toggles
   - verify rolling bar playback updates

## PR checklist

- [ ] Change has clear scope and rationale.
- [ ] Relevant tests updated or added.
- [ ] `ctest` passes locally.
- [ ] No new warnings/errors introduced in edited files.
- [ ] README/Developer Notes updated if behavior changed.

## Style notes

- Use C++17 compatible code.
- Prefer small, composable helper functions over deeply nested blocks.
- Keep new comments short and only for non-obvious logic.
