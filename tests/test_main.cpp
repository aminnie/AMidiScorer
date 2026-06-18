#include <JuceHeader.h>
#include "../src/midi/TempoMap.h"
#include "../src/midi/TrackNoteExtractor.h"
#include "../src/notation/Quantizer.h"
#include "../src/harmony/ChordDetector.h"

namespace
{
int failures = 0;

void expectTrue(bool condition, const juce::String& label)
{
    if (!condition)
    {
        ++failures;
        juce::Logger::writeToLog("FAIL: " + label);
    }
    else
    {
        juce::Logger::writeToLog("PASS: " + label);
    }
}

void testTempoMap()
{
    TempoMap map;
    std::vector<TempoMetaEvent> tempos { { 0.0, 120.0 }, { 960.0, 60.0 } };
    std::vector<TimeSignatureMetaEvent> signatures { { 0.0, 4, 4 }, { 1920.0, 3, 4 } };
    expectTrue(map.build(960.0, tempos, signatures, 3840.0), "TempoMap builds");
    expectTrue(std::abs(map.tickToSeconds(960.0) - 0.5) < 0.02, "Tick->seconds uses tempo");
    expectTrue(map.quarterToBar(0.0) == 1, "Quarter zero is bar 1");
    expectTrue(map.quarterToBar(4.0) == 2, "Bar increments in 4/4");
}

void testQuantizer()
{
    TempoMap map;
    std::vector<TempoMetaEvent> tempos { { 0.0, 120.0 } };
    std::vector<TimeSignatureMetaEvent> signatures { { 0.0, 4, 4 } };
    map.build(960.0, tempos, signatures, 1920.0);

    MidiNoteEvent note;
    note.noteNumber = 64;
    note.startTick = 0.0;
    note.endTick = 960.0;
    std::vector<MidiNoteEvent> notes { note };

    auto quantized = Quantizer::quantizeTrack(notes, map);
    expectTrue(!quantized.empty(), "Quantizer returns notes");
    expectTrue(quantized.front().value == NoteValue::quarter, "Quarter duration detected");
}

void testChordDetector()
{
    TempoMap map;
    std::vector<TempoMetaEvent> tempos { { 0.0, 120.0 } };
    std::vector<TimeSignatureMetaEvent> signatures { { 0.0, 4, 4 } };
    map.build(960.0, tempos, signatures, 3840.0);

    std::vector<MidiNoteEvent> notes;
    for (int midiNote : { 60, 64, 67, 71, 74 })
    {
        MidiNoteEvent ev;
        ev.noteNumber = midiNote;
        ev.startSec = 0.0;
        ev.endSec = 2.0;
        notes.push_back(ev);
    }

    auto chords = ChordDetector::detect(notes, map, 2);
    expectTrue(!chords.empty(), "Chord detector returns at least one chord");
    if (!chords.empty())
        expectTrue(chords.front().symbol.startsWith("C"), "Chord detector resolves C-root harmony");

    ChordDetector::NamingOptions flatOptions;
    flatOptions.accidentalPreference = ChordDetector::AccidentalPreference::preferFlats;
    flatOptions.jazzAliasStyle = ChordDetector::JazzAliasStyle::jazzSymbols;
    auto flatChords = ChordDetector::detect(notes, map, 2, flatOptions);
    expectTrue(!flatChords.empty(), "Chord detector supports naming options");
    if (!flatChords.empty())
        expectTrue(flatChords.front().symbol.contains("C"), "Chord naming options keep root identity");
}
}

int main()
{
    testTempoMap();
    testQuantizer();
    testChordDetector();

    if (failures == 0)
    {
        juce::Logger::writeToLog("All MidiScorer tests passed.");
        return 0;
    }

    juce::Logger::writeToLog("MidiScorer tests failed: " + juce::String(failures));
    return 1;
}
