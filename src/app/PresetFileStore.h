#pragma once

#include <JuceHeader.h>
#include <functional>

class PresetFileStore
{
public:
    static juce::File getPresetFilePath()
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("MidiScorer");
        if (!dir.exists())
            dir.createDirectory();
        return dir.getChildFile("ui_preset.json");
    }

    static juce::File getMidiOutputConfigPath()
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("MidiScorer");
        if (!dir.exists())
            dir.createDirectory();
        return dir.getChildFile("midi_output.json");
    }

    static juce::String songKeyFromProjectFile(const juce::File& projectFile)
    {
        if (!projectFile.existsAsFile())
            return {};
        return projectFile.getFullPathName().replaceCharacter('\\', '/').toLowerCase();
    }

    static bool writeTextFileAtomically(const juce::File& targetFile, const juce::String& text, juce::String& error)
    {
        juce::TemporaryFile tempFile(targetFile);
        if (!tempFile.getFile().replaceWithText(text))
        {
            error = "unable to write temporary file";
            return false;
        }
        if (!tempFile.overwriteTargetFileWithTemporary())
        {
            error = "unable to replace target file";
            return false;
        }
        return true;
    }

    static bool mergeWritePreset(const std::function<void(juce::DynamicObject&)>& mutate, juce::String& error)
    {
        const auto file = getPresetFilePath();
        juce::var parsed;
        auto obj = std::make_unique<juce::DynamicObject>();

        if (file.existsAsFile())
        {
            const auto parseResult = juce::JSON::parse(file.loadFileAsString(), parsed);
            if (!parseResult.failed() && parsed.isObject())
            {
                if (auto* existing = parsed.getDynamicObject())
                {
                    for (const auto& property : existing->getProperties())
                        obj->setProperty(property.name, property.value);
                }
            }
        }

        mutate(*obj);
        return writeTextFileAtomically(file, juce::JSON::toString(juce::var(obj.release())), error);
    }
};
