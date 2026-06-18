#pragma once

#include <JuceHeader.h>

class PlaybackClock
{
public:
    void start(double fromSecond = 0.0)
    {
        offsetSec = juce::jmax(0.0, fromSecond);
        startMs = juce::Time::getMillisecondCounterHiRes();
        playing = true;
    }

    void pause()
    {
        if (!playing)
            return;

        offsetSec = getElapsedSeconds();
        playing = false;
        startMs = 0.0;
    }

    void stop()
    {
        playing = false;
        startMs = 0.0;
        offsetSec = 0.0;
    }

    void seek(double second)
    {
        offsetSec = juce::jmax(0.0, second);
        if (playing)
            startMs = juce::Time::getMillisecondCounterHiRes();
    }

    double getElapsedSeconds() const
    {
        if (!playing)
            return offsetSec;

        const auto now = juce::Time::getMillisecondCounterHiRes();
        const auto delta = juce::jmax(0.0, (now - startMs) / 1000.0);
        return offsetSec + delta;
    }

    bool isPlaying() const { return playing; }

private:
    bool playing = false;
    double startMs = 0.0;
    double offsetSec = 0.0;
};
