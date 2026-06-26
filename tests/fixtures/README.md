# MIDI Fixtures

This folder stores deterministic fixture definitions used for verification:

- `fixture-specs.md` documents exact test scenarios.
- `generate_fixtures.py` regenerates the checked-in `.mid` files from those specs (requires `mido`: `python -m pip install mido`).
- `tempo_time_sig.mid` is a checked-in minimal fixture for loader/meta timing integration coverage.
- `ties_syncopation.mid` is a checked-in minimal fixture for note/tie ingest coverage.
