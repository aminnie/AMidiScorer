#pragma once

#include <JuceHeader.h>
#include <optional>

namespace WorkingDirectoryCopy
{
inline bool needsCopyToWorkingDirectory(const juce::File& sourceFile, const juce::File& workingDirectory)
{
    if (!sourceFile.existsAsFile() || !workingDirectory.isDirectory())
        return false;

    return !sourceFile.getParentDirectory().getFullPathName().equalsIgnoreCase(workingDirectory.getFullPathName());
}

inline juce::String normalizeMidiBaseName(const juce::String& input)
{
    juce::String result = input.trim();
    const auto lowered = result.toLowerCase();
    if (lowered.endsWith(".midi"))
        result = result.dropLastCharacters(5);
    else if (lowered.endsWith(".mid"))
        result = result.dropLastCharacters(4);

    result = result.removeCharacters("\\/:*?\"<>|");

    while (result.isNotEmpty() && (result.endsWithChar('.') || result.endsWithChar(' ')))
        result = result.dropLastCharacters(1);
    while (result.isNotEmpty() && result.startsWithChar(' '))
        result = result.substring(1);

    return result.trim();
}

inline juce::String normalizeMidiExtension(const juce::String& extension)
{
    juce::String normalized = extension.trim().toLowerCase();
    if (!normalized.startsWithChar('.'))
        normalized = "." + normalized;

    if (normalized == ".mid" || normalized == ".midi")
        return normalized;

    return ".mid";
}

inline std::optional<juce::File> buildWorkingDirectoryDestination(const juce::File& workingDirectory,
                                                                  const juce::String& requestedBaseName,
                                                                  const juce::String& sourceExtension,
                                                                  juce::String& error)
{
    error = {};
    if (!workingDirectory.isDirectory())
    {
        error = "Working directory is not available.";
        return std::nullopt;
    }

    const auto baseName = normalizeMidiBaseName(requestedBaseName);
    if (baseName.isEmpty())
    {
        error = "Please enter a valid MIDI filename.";
        return std::nullopt;
    }

    const auto extension = normalizeMidiExtension(sourceExtension);
    const auto destination = workingDirectory.getChildFile(baseName + extension);
    if (destination.existsAsFile())
    {
        error = "A file with that name already exists in the working directory.";
        return std::nullopt;
    }

    return destination;
}
}
