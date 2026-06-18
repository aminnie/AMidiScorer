#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include <vector>
#include "../midi/TempoMap.h"
#include "Quantizer.h"

struct ChordAnnotation
{
    int barNumber = 1;
    double quarter = 0.0;
    juce::String symbol;
};

struct ScoreNoteSymbol
{
    int barNumber = 1;
    double quarter = 0.0;
    double quarterInBar = 0.0;
    int midiNote = 60;
    NoteValue value = NoteValue::quarter;
    bool tieIntoNextBar = false;
};

struct ScoreBar
{
    int barNumber = 1;
    int numerator = 4;
    int denominator = 4;
    std::vector<ScoreNoteSymbol> notes;
    std::vector<ChordAnnotation> chords;
};

class ScoreModel
{
public:
    void clear()
    {
        bars.clear();
        firstBar = 1;
        lastBar = 1;
    }

    void build(const TempoMap& map,
               const std::vector<QuantizedNote>& quantizedNotes,
               const std::vector<ChordAnnotation>& chordEvents,
               int maxBarHint)
    {
        clear();

        firstBar = 1;
        lastBar = juce::jmax(1, maxBarHint);
        bars.reserve(static_cast<size_t>(lastBar));

        for (int bar = firstBar; bar <= lastBar; ++bar)
        {
            ScoreBar scoreBar;
            scoreBar.barNumber = bar;
            const auto signature = signatureForBar(map, bar);
            scoreBar.numerator = signature.first;
            scoreBar.denominator = signature.second;
            bars.push_back(scoreBar);
        }

        for (const auto& n : quantizedNotes)
        {
            const int bar = map.quarterToBar(n.startQuarter);
            const int idx = barToIndex(bar);
            if (!isValidIndex(idx))
                continue;

            ScoreNoteSymbol symbol;
            symbol.barNumber = bar;
            symbol.quarter = n.startQuarter;
            symbol.quarterInBar = juce::jmax(0.0, n.startQuarter - map.barToQuarterDownbeat(bar));
            symbol.midiNote = n.midiNote;
            symbol.value = n.value;

            const auto endQuarter = n.startQuarter + n.durationQuarter;
            const int endBar = map.quarterToBar(endQuarter - 1.0e-6);
            symbol.tieIntoNextBar = endBar > bar;
            bars[(size_t) idx].notes.push_back(symbol);
        }

        for (const auto& chord : chordEvents)
        {
            const int idx = barToIndex(chord.barNumber);
            if (!isValidIndex(idx))
                continue;

            bars[(size_t) idx].chords.push_back(chord);
        }

        for (auto& bar : bars)
        {
            std::sort(bar.notes.begin(), bar.notes.end(), [](const auto& a, const auto& b) { return a.quarter < b.quarter; });
            std::sort(bar.chords.begin(), bar.chords.end(), [](const auto& a, const auto& b) { return a.quarter < b.quarter; });
        }
    }

    std::vector<ScoreBar> getWindowBars(int centerBar, int left = 2, int right = 2) const
    {
        std::vector<ScoreBar> out;
        if (bars.empty())
            return out;

        const auto from = juce::jmax(firstBar, centerBar - left);
        const auto to = juce::jmin(lastBar, centerBar + right);
        out.reserve(static_cast<size_t>(juce::jmax(0, to - from + 1)));

        for (int b = from; b <= to; ++b)
        {
            const auto idx = barToIndex(b);
            if (isValidIndex(idx))
                out.push_back(bars[(size_t) idx]);
        }

        return out;
    }

    int getFirstBar() const { return firstBar; }
    int getLastBar() const { return lastBar; }
    bool empty() const { return bars.empty(); }

private:
    std::pair<int, int> signatureForBar(const TempoMap& map, int barNumber) const
    {
        const auto q = map.barToQuarterDownbeat(barNumber);
        const auto& sigs = map.getTimeSignatureEvents();
        if (sigs.empty())
            return { 4, 4 };

        int idx = 0;
        while (idx + 1 < static_cast<int>(sigs.size()) && sigs[(size_t) (idx + 1)].quarterAtTick <= q)
            ++idx;

        return { sigs[(size_t) idx].numerator, sigs[(size_t) idx].denominator };
    }

    int barToIndex(int barNumber) const
    {
        return barNumber - firstBar;
    }

    bool isValidIndex(int index) const
    {
        return index >= 0 && index < static_cast<int>(bars.size());
    }

    int firstBar = 1;
    int lastBar = 1;
    std::vector<ScoreBar> bars;
};
