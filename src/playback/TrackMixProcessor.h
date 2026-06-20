#pragma once

#include <JuceHeader.h>
#include <cmath>
#include "TrackMixState.h"

class TrackMixProcessor
{
public:
    static bool shouldSendTrack(int trackIndex, const TrackMixState& mixState)
    {
        if (!mixState.isValidTrack(trackIndex))
            return true;

        if (mixState.anySoloActive())
            return mixState.isSolo(trackIndex);

        return !mixState.isMuted(trackIndex);
    }

    static juce::MidiMessage applyVolumeToMessage(const juce::MidiMessage& message,
                                                  int trackIndex,
                                                  const TrackMixState& mixState)
    {
        if (!mixState.isValidTrack(trackIndex))
            return message;

        const int trackVolume = mixState.getVolume(trackIndex);
        const float gain = static_cast<float>(trackVolume) / 127.0f;

        if (message.isNoteOn())
        {
            const int scaledVelocity = juce::jlimit(0, 127, static_cast<int>(std::round(message.getVelocity() * gain)));
            auto transformed = juce::MidiMessage::noteOn(message.getChannel(),
                                                         message.getNoteNumber(),
                                                         static_cast<juce::uint8>(scaledVelocity));
            transformed.setTimeStamp(message.getTimeStamp());
            return transformed;
        }

        if (message.isController())
        {
            const int controller = message.getControllerNumber();
            if (controller == 7 || controller == 11)
            {
                const int scaledValue = juce::jlimit(0, 127, static_cast<int>(std::round(message.getControllerValue() * gain)));
                auto transformed = juce::MidiMessage::controllerEvent(message.getChannel(), controller, scaledValue);
                transformed.setTimeStamp(message.getTimeStamp());
                return transformed;
            }
        }

        return message;
    }
};
