#!/usr/bin/env python3
"""Regenerate checked-in MIDI fixtures from tests/fixtures/fixture-specs.md."""

from pathlib import Path

import mido
from mido import Message, MetaMessage, MidiFile, MidiTrack

FIXTURES_DIR = Path(__file__).resolve().parent


def write_tempo_time_sig(path: Path) -> None:
    mid = MidiFile(type=1, ticks_per_beat=480)
    track = MidiTrack()
    mid.tracks.append(track)

    track.append(MetaMessage("set_tempo", tempo=500000, time=0))
    track.append(MetaMessage("time_signature", numerator=4, denominator=4, time=0))
    track.append(Message("note_on", channel=0, note=60, velocity=80, time=0))
    track.append(MetaMessage("time_signature", numerator=3, denominator=4, time=3840))
    track.append(MetaMessage("set_tempo", tempo=666666, time=1440))
    track.append(Message("note_off", channel=0, note=60, velocity=0, time=1920))
    track.append(Message("note_on", channel=0, note=62, velocity=80, time=0))
    track.append(Message("note_off", channel=0, note=62, velocity=0, time=2880))
    track.append(MetaMessage("end_of_track", time=0))

    mid.save(path)


def write_ties_syncopation(path: Path) -> None:
    mid = MidiFile(type=1, ticks_per_beat=480)
    track = MidiTrack()
    mid.tracks.append(track)

    track.append(MetaMessage("set_tempo", tempo=500000, time=0))
    track.append(MetaMessage("time_signature", numerator=4, denominator=4, time=0))
    track.append(Message("note_on", channel=0, note=60, velocity=90, time=1440))
    track.append(Message("note_off", channel=0, note=60, velocity=0, time=2400))
    track.append(Message("note_on", channel=0, note=64, velocity=80, time=240))
    track.append(Message("note_off", channel=0, note=64, velocity=0, time=480))
    track.append(MetaMessage("end_of_track", time=0))

    mid.save(path)


def main() -> None:
    write_tempo_time_sig(FIXTURES_DIR / "tempo_time_sig.mid")
    write_ties_syncopation(FIXTURES_DIR / "ties_syncopation.mid")
    print(f"Wrote fixtures to {FIXTURES_DIR}")


if __name__ == "__main__":
    main()
