#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>
#include "MainComponent.h"

class PlayerTabComponent final : public juce::Component,
                                 private juce::Timer
{
public:
    explicit PlayerTabComponent(MainComponent& scorePageRef)
        : scorePage(scorePageRef)
    {
        addAndMakeVisible(fileLabel);
        fileLabel.setJustificationType(juce::Justification::centredLeft);

        addAndMakeVisible(statusLabel);
        statusLabel.setJustificationType(juce::Justification::centredLeft);

        addAndMakeVisible(outputLabel);
        outputLabel.setText("MIDI Output", juce::dontSendNotification);
        outputLabel.setJustificationType(juce::Justification::centredRight);

        addAndMakeVisible(refreshOutputsButton);
        refreshOutputsButton.setButtonText("Refresh");
        refreshOutputsButton.onClick = [this] { refreshOutputDeviceList(); };

        addAndMakeVisible(startupResumeToggle);
        startupResumeToggle.setButtonText("Reopen last MIDI on startup");
        startupResumeToggle.setToggleState(scorePage.isStartupResumeEnabled(), juce::dontSendNotification);
        startupResumeToggle.setTooltip("When enabled, MidiScorer reopens the last loaded MIDI file (or most recent file) on launch.");
        startupResumeToggle.onClick = [this]
        {
            scorePage.setStartupResumeEnabled(startupResumeToggle.getToggleState());
        };

        addAndMakeVisible(workingDirectoryLabel);
        workingDirectoryLabel.setText("Working Directory", juce::dontSendNotification);
        workingDirectoryLabel.setJustificationType(juce::Justification::centredRight);

        addAndMakeVisible(workingDirectoryInput);
        workingDirectoryInput.setText(scorePage.getWorkingDirectoryPath(), juce::dontSendNotification);
        workingDirectoryInput.setTooltip("MIDI files loaded from outside this folder are copied here automatically.");
        workingDirectoryInput.onReturnKey = [this] { applyWorkingDirectoryInput(); };
        workingDirectoryInput.onFocusLost = [this] { applyWorkingDirectoryInput(); };

        addAndMakeVisible(browseWorkingDirectoryButton);
        browseWorkingDirectoryButton.setButtonText("Set Working Directory...");
        browseWorkingDirectoryButton.onClick = [this] { browseWorkingDirectory(); };

        addAndMakeVisible(addWorkingSubfolderButton);
        addWorkingSubfolderButton.setButtonText("Add Subfolder...");
        addWorkingSubfolderButton.setTooltip("Create a new folder under the selected working directory.");
        addWorkingSubfolderButton.onClick = [this] { addWorkingSubfolder(); };

        addAndMakeVisible(scoreLightModeToggle);
        scoreLightModeToggle.setButtonText("Light Score");
        scoreLightModeToggle.setToggleState(scorePage.isScoreLightMode(), juce::dontSendNotification);
        scoreLightModeToggle.setTooltip("Toggle score display between white-on-black and black-on-white.");
        scoreLightModeToggle.onClick = [this]
        {
            scorePage.setScoreLightMode(scoreLightModeToggle.getToggleState());
        };

        addAndMakeVisible(outputSelector);
        outputSelector.onChange = [this] { handleOutputSelectionChanged(); };

        addAndMakeVisible(exitButton);
        exitButton.setButtonText("Exit");
        exitButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        exitButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        exitButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black.darker());
        exitButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::black.brighter());
        exitButton.onClick = [this]()
        {
            if (exitAction)
                exitAction();
        };

        refreshOutputDeviceList();
        refreshLiveStatus();
        startTimerHz(5);
    }

    void setExitAction(std::function<void()> action)
    {
        exitAction = std::move(action);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(12);
        auto row1 = area.removeFromTop(28);
        outputLabel.setBounds(row1.removeFromLeft(90));
        outputSelector.setBounds(row1.removeFromLeft(360).reduced(4, 0));
        refreshOutputsButton.setBounds(row1.removeFromLeft(96).reduced(4, 0));
        exitButton.setBounds(row1.removeFromRight(80).reduced(4, 0));

        area.removeFromTop(8);
        auto workingRow = area.removeFromTop(24);
        workingDirectoryLabel.setBounds(workingRow.removeFromLeft(116));
        workingDirectoryInput.setBounds(workingRow.removeFromLeft(470).reduced(4, 0));
        browseWorkingDirectoryButton.setBounds(workingRow.removeFromLeft(188).reduced(4, 0));

        area.removeFromTop(6);
        auto subfolderRow = area.removeFromTop(24);
        subfolderRow.removeFromLeft(590);
        addWorkingSubfolderButton.setBounds(subfolderRow.removeFromLeft(188).reduced(4, 0));

        area.removeFromTop(8);
        auto toggleRow = area.removeFromTop(24);
        startupResumeToggle.setBounds(toggleRow.removeFromLeft(260));
        scoreLightModeToggle.setBounds(toggleRow.removeFromLeft(120));

        area.removeFromTop(8);
        fileLabel.setBounds(area.removeFromTop(24));

        area.removeFromTop(8);
        statusLabel.setBounds(area.removeFromTop(24));
    }

private:
    void timerCallback() override
    {
        refreshLiveStatus();
    }

    void refreshOutputDeviceList()
    {
        availableOutputs = scorePage.getAvailableMidiOutputs();
        outputSelector.clear(juce::dontSendNotification);
        outputSelector.addItem("(None)", 1);

        int itemId = 2;
        int selectedId = 1;
        const auto currentSelection = scorePage.getSelectedMidiOutputIdentifier();
        for (const auto& output : availableOutputs)
        {
            outputSelector.addItem(output.name, itemId);
            if (output.identifier == currentSelection)
                selectedId = itemId;
            ++itemId;
        }

        outputSelector.setSelectedId(selectedId, juce::dontSendNotification);
    }

    void handleOutputSelectionChanged()
    {
        const int selectedId = outputSelector.getSelectedId();
        if (selectedId <= 1)
        {
            scorePage.clearMidiOutputDevice();
            refreshLiveStatus();
            return;
        }

        const int index = selectedId - 2;
        if (index < 0 || index >= static_cast<int>(availableOutputs.size()))
            return;

        juce::String error;
        const auto selectedIdentifier = availableOutputs[(size_t) index].identifier;
        const bool selected = scorePage.selectMidiOutputDevice(selectedIdentifier, true, error);
        if (!selected)
            statusLabel.setText("MIDI output error: " + error, juce::dontSendNotification);
    }

    void refreshLiveStatus()
    {
        const auto midiName = scorePage.getLoadedMidiFileName();
        if (midiName.isNotEmpty())
        {
            juce::String text = "Loaded MIDI: " + midiName;
            if (scorePage.isLoadedMidiInWorkingDirectory())
                text << " (Working Directory)";
            else
                text << " (Outside Working Directory)";
            fileLabel.setText(text, juce::dontSendNotification);
        }
        else
        {
            fileLabel.setText("Loaded MIDI: none", juce::dontSendNotification);
        }

        const auto workingDirPath = scorePage.getWorkingDirectoryPath();
        if (workingDirectoryInput.getText() != workingDirPath)
            workingDirectoryInput.setText(workingDirPath, juce::dontSendNotification);

        const bool isPlaying = scorePage.isPlaybackRunning();
        const int currentBar = scorePage.getCurrentPlaybackBar();
        const int maxBar = scorePage.getMaximumBar();
        auto outputName = scorePage.getSelectedMidiOutputName();
        if (outputName.isEmpty())
            outputName = "(None)";

        juce::String statusText = juce::String(isPlaying ? "Playing" : "Stopped")
            + "  | Bar " + juce::String(currentBar) + "/" + juce::String(maxBar)
            + "  | Output: " + outputName;
        const auto restoreWarning = scorePage.getMidiOutputRestoreWarning();
        if (restoreWarning.isNotEmpty())
            statusText << "  | Warning: " << restoreWarning;
        statusLabel.setText(statusText, juce::dontSendNotification);

        const bool lightScore = scorePage.isScoreLightMode();
        if (scoreLightModeToggle.getToggleState() != lightScore)
            scoreLightModeToggle.setToggleState(lightScore, juce::dontSendNotification);
    }

    void applyWorkingDirectoryInput()
    {
        juce::String error;
        if (!scorePage.setWorkingDirectoryPath(workingDirectoryInput.getText(), error))
        {
            statusLabel.setText("Working directory error: " + error, juce::dontSendNotification);
            workingDirectoryInput.setText(scorePage.getWorkingDirectoryPath(), juce::dontSendNotification);
            return;
        }

        workingDirectoryInput.setText(scorePage.getWorkingDirectoryPath(), juce::dontSendNotification);
    }

    void browseWorkingDirectory()
    {
        workingDirectoryChooser = std::make_unique<juce::FileChooser>(
            "Set working directory",
            juce::File(scorePage.getWorkingDirectoryPath()),
            juce::String(),
            false,
            false,
            this);
        const auto chooserFlags = juce::FileBrowserComponent::openMode
                                | juce::FileBrowserComponent::canSelectDirectories;
        workingDirectoryChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& chooser)
        {
            auto selected = chooser.getResult();
            if (!selected.exists())
                return;

            if (!selected.isDirectory())
                return;

            workingDirectoryInput.setText(selected.getFullPathName(), juce::dontSendNotification);
            applyWorkingDirectoryInput();
        });
    }

    static juce::String sanitizeFolderName(const juce::String& input)
    {
        auto name = input.trim().removeCharacters("\\/:*?\"<>|");
        while (name.isNotEmpty() && (name.endsWithChar('.') || name.endsWithChar(' ')))
            name = name.dropLastCharacters(1);
        return name.trim();
    }

    void addWorkingSubfolder()
    {
        const juce::File parentDirectory(scorePage.getWorkingDirectoryPath());
        if (!parentDirectory.isDirectory())
        {
            statusLabel.setText("Working directory error: Select a valid working directory first.", juce::dontSendNotification);
            return;
        }

        auto* alert = new juce::AlertWindow("Add Subfolder",
                                            "Create a new folder inside:\n" + parentDirectory.getFullPathName(),
                                            juce::MessageBoxIconType::QuestionIcon,
                                            this);
        alert->addTextEditor("subfolderName", "NewFolder", "Folder name");
        alert->addButton("Create", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alert->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        juce::Component::SafePointer<PlayerTabComponent> safeThis(this);
        alert->enterModalState(true, juce::ModalCallbackFunction::create([safeThis, alert, parentDirectory](int result)
        {
            if (safeThis == nullptr)
                return;

            if (result != 1)
                return;

            const auto requested = alert->getTextEditorContents("subfolderName");
            const auto folderName = sanitizeFolderName(requested);
            if (folderName.isEmpty())
            {
                safeThis->statusLabel.setText("Working directory error: Enter a valid folder name.", juce::dontSendNotification);
                return;
            }

            const juce::File newFolder = parentDirectory.getChildFile(folderName);
            if (newFolder.exists())
            {
                safeThis->statusLabel.setText("Working directory error: That folder already exists.", juce::dontSendNotification);
                return;
            }

            if (!newFolder.createDirectory())
            {
                safeThis->statusLabel.setText("Working directory error: Could not create folder.", juce::dontSendNotification);
                return;
            }

            safeThis->workingDirectoryInput.setText(newFolder.getFullPathName(), juce::dontSendNotification);
            safeThis->applyWorkingDirectoryInput();
        }), true);
    }

    MainComponent& scorePage;
    std::vector<MidiOutputDeviceInfo> availableOutputs;
    std::function<void()> exitAction;
    std::unique_ptr<juce::FileChooser> workingDirectoryChooser;

    juce::Label outputLabel;
    juce::ComboBox outputSelector;
    juce::TextButton refreshOutputsButton;
    juce::TextButton exitButton { "Exit" };
    juce::ToggleButton startupResumeToggle;
    juce::Label workingDirectoryLabel;
    juce::TextEditor workingDirectoryInput;
    juce::TextButton browseWorkingDirectoryButton;
    juce::TextButton addWorkingSubfolderButton;
    juce::ToggleButton scoreLightModeToggle;
    juce::Label fileLabel;
    juce::Label statusLabel;
};
