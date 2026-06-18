#pragma once

#include <JuceHeader.h>
#include <array>
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
};

class Quantizer
{
public:
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
            q.value = quarterToValue(q.durationQuarter);
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
        static constexpr std::array<double, 5> choices { 0.25, 0.5, 1.0, 2.0, 4.0 };
        const auto clamped = juce::jlimit(0.125, 8.0, quarterLength);

        double best = choices.front();
        double bestDist = std::numeric_limits<double>::max();
        for (double choice : choices)
        {
            const auto d = std::abs(choice - clamped);
            if (d < bestDist)
            {
                bestDist = d;
                best = choice;
            }
        }
        return best;
    }

    static NoteValue quarterToValue(double quarterLength)
    {
        if (quarterLength <= 0.25 + 1.0e-6)
            return NoteValue::sixteenth;
        if (quarterLength <= 0.5 + 1.0e-6)
            return NoteValue::eighth;
        if (quarterLength <= 1.0 + 1.0e-6)
            return NoteValue::quarter;
        if (quarterLength <= 2.0 + 1.0e-6)
            return NoteValue::half;
        return NoteValue::whole;
    }
};
