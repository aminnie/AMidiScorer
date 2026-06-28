#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <vector>
#include "../midi/TempoMap.h"
#include "../midi/TrackNoteExtractor.h"

enum class NoteValue
{
    sixteenth,
    eighth,
    quarter,
    half,
    whole
};

struct QuantizedNote
{
    int midiNote = 60;
    int velocity = 100;
    int channel = 1;
    double startQuarter = 0.0;
    double durationQuarter = 1.0;
    NoteValue value = NoteValue::quarter;
    bool dotted = false;
};

class Quantizer
{
public:
    struct DurationSymbol
    {
        NoteValue value = NoteValue::quarter;
        bool dotted = false;
    };

    static DurationSymbol durationFromQuarter(double quarterLength)
    {
        struct Choice
        {
            double quarter;
            NoteValue value;
            bool dotted;
        };

        static constexpr Choice choices[] {
            { 0.25, NoteValue::sixteenth, false },
            { 0.375, NoteValue::sixteenth, true },
            { 0.5, NoteValue::eighth, false },
            { 0.75, NoteValue::eighth, true },
            { 1.0, NoteValue::quarter, false },
            { 1.5, NoteValue::quarter, true },
            { 2.0, NoteValue::half, false },
            { 3.0, NoteValue::half, true },
            { 4.0, NoteValue::whole, false },
            { 6.0, NoteValue::whole, true },
        };

        const auto clamped = juce::jlimit(0.125, 8.0, quarterLength);
        DurationSymbol best;
        double bestDist = std::numeric_limits<double>::max();
        for (const auto& choice : choices)
        {
            const auto distance = std::abs(choice.quarter - clamped);
            if (distance < bestDist)
            {
                bestDist = distance;
                best.value = choice.value;
                best.dotted = choice.dotted;
            }
        }
        return best;
    }

    static double quarterFromDuration(NoteValue value, bool dotted)
    {
        double base = 1.0;
        switch (value)
        {
            case NoteValue::sixteenth: base = 0.25; break;
            case NoteValue::eighth: base = 0.5; break;
            case NoteValue::quarter: base = 1.0; break;
            case NoteValue::half: base = 2.0; break;
            case NoteValue::whole: base = 4.0; break;
        }
        return dotted ? base * 1.5 : base;
    }
    static std::vector<QuantizedNote> quantizeTrack(const std::vector<MidiNoteEvent>& notes, const TempoMap& map)
    {
        std::vector<QuantizedNote> out;
        out.reserve(notes.size());

        for (const auto& note : notes)
        {
            QuantizedNote q;
            q.midiNote = note.noteNumber;
            q.velocity = note.velocity;
            q.channel = note.channel;

            const auto rawStartQuarter = map.tickToQuarter(note.startTick);
            const auto rawEndQuarter = map.tickToQuarter(note.endTick);
            const auto rawDurationQuarter = juce::jmax(0.0, rawEndQuarter - rawStartQuarter);

            q.startQuarter = quantizeToSixteenth(rawStartQuarter);
            q.durationQuarter = quantizeDuration(rawDurationQuarter);
            const auto durationSymbol = durationFromQuarter(q.durationQuarter);
            q.value = durationSymbol.value;
            q.dotted = durationSymbol.dotted;
            out.push_back(q);
        }

        return out;
    }

    static double quantizeToSixteenth(double quarterPos)
    {
        const double grid = 0.25;
        return std::round(quarterPos / grid) * grid;
    }

    static double quantizeDuration(double quarterLength)
    {
        const auto symbol = durationFromQuarter(quarterLength);
        return quarterFromDuration(symbol.value, symbol.dotted);
    }

    static NoteValue quarterToValue(double quarterLength)
    {
        return durationFromQuarter(quarterLength).value;
    }
};
