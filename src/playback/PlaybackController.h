#pragma once

#include <JuceHeader.h>
#include "PlaybackClock.h"
#include "../midi/TempoMap.h"

class PlaybackController
{
public:
    void setTempoMap(const TempoMap* mapPtr, double durationSec)
    {
        tempoMap = mapPtr;
        totalDurationSec = juce::jmax(0.0, durationSec);
        clock.stop();
    }

    void playFromStart()
    {
        clock.start(0.0);
    }

    void playFromSecond(double second)
    {
        const auto clamped = totalDurationSec > 0.0
            ? juce::jlimit(0.0, totalDurationSec, juce::jmax(0.0, second))
            : juce::jmax(0.0, second);
        clock.start(clamped);
    }

    void playFromBar(int barNumber)
    {
        if (tempoMap == nullptr)
        {
            playFromStart();
            return;
        }

        const int normalizedBar = juce::jmax(1, barNumber);
        playFromSecond(tempoMap->barToSecondsDownbeat(normalizedBar));
    }

    void stop()
    {
        clock.stop();
    }

    bool isPlaying() const
    {
        return clock.isPlaying();
    }

    double getElapsedSeconds() const
    {
        return clock.getElapsedSeconds();
    }

    int getCurrentBar() const
    {
        if (tempoMap == nullptr)
            return 1;
        return tempoMap->secondsToBar(getElapsedSeconds());
    }

    bool hasReachedEnd() const
    {
        return totalDurationSec > 0.0 && getElapsedSeconds() >= totalDurationSec;
    }

private:
    PlaybackClock clock;
    const TempoMap* tempoMap = nullptr;
    double totalDurationSec = 0.0;
};
