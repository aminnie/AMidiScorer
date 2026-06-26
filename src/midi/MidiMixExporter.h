#pragma once

#include <JuceHeader.h>
#include <cmath>
#include "MidiProjectLoader.h"
#include "../playback/TrackMixProcessor.h"
#include "../playback/TrackMixState.h"

class MidiMixExporter
{
public:
    static bool exportMixedType1File(const MidiProjectData& project,
                                     const TrackMixState& mixState,
                                     const juce::File& outFile,
                                     juce::String& error)
    {
        error.clear();

        if (project.tracks.empty())
        {
            error = "No loaded MIDI project to export.";
            return false;
        }

        juce::MidiFile outMidi;
        outMidi.setTicksPerQuarterNote(static_cast<int>(std::round(project.tempoMap.getPPQ())));

        for (int trackIndex = 0; trackIndex < static_cast<int>(project.tracks.size()); ++trackIndex)
        {
            const auto& srcTrack = project.tracks[(size_t) trackIndex];
            juce::MidiMessageSequence outTrack;

            for (int eventIndex = 0; eventIndex < srcTrack.sequence.getNumEvents(); ++eventIndex)
            {
                const auto* holder = srcTrack.sequence.getEventPointer(eventIndex);
                if (holder == nullptr)
                    continue;

                const auto& message = holder->message;

                if (message.isMetaEvent() || message.isSysEx())
                {
                    outTrack.addEvent(message, message.getTimeStamp());
                    continue;
                }

                if (!TrackMixProcessor::shouldSendTrack(trackIndex, mixState))
                    continue;

                auto mixed = TrackMixProcessor::applyVolumeToMessage(message, trackIndex, mixState);
                mixed.setTimeStamp(message.getTimeStamp());
                outTrack.addEvent(mixed, mixed.getTimeStamp());
            }

            if (outTrack.getNumEvents() == 0)
                continue;

            outTrack.updateMatchedPairs();
            outMidi.addTrack(outTrack);
        }

        if (outMidi.getNumTracks() == 0)
        {
            error = "All tracks were muted/filtered. Nothing to export.";
            return false;
        }

        juce::FileOutputStream stream(outFile);
        if (!stream.openedOk())
        {
            error = "Failed to open export destination: " + outFile.getFullPathName();
            return false;
        }

        if (!outMidi.writeTo(stream))
        {
            error = "Failed to write mixed MIDI file.";
            return false;
        }

        return true;
    }
};
