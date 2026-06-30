#pragma once

#include <JuceHeader.h>
#include "MainComponent.h"
#include "PlayerTabComponent.h"
#include "TracksTabComponent.h"

enum AppTabIndex
{
    PTStart = 0,
    PTScore = 1,
    PTEffects = 2,
    PTExit = 3
};

class AppTabbedComponent final : public juce::TabbedComponent
{
public:
    AppTabbedComponent()
        : juce::TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
    {
    }

    void currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName) override
    {
        if (newCurrentTabName == "Exit")
        {
            setCurrentTabIndex(lastNonExitTabIndex, false);

            auto closeApplication = [safeTop = juce::Component::SafePointer<juce::Component>(getTopLevelComponent())]()
            {
                if (safeTop != nullptr)
                {
                    juce::MessageManager::callAsync([safeTop]()
                    {
                        if (safeTop != nullptr)
                            safeTop->userTriedToCloseWindow();
                    });
                }
                else if (auto* app = juce::JUCEApplication::getInstance())
                {
                    juce::MessageManager::callAsync([app]() { app->systemRequestedQuit(); });
                }
            };
            closeApplication();
            return;
        }

        lastNonExitTabIndex = newCurrentTabIndex;
        juce::TabbedComponent::currentTabChanged(newCurrentTabIndex, newCurrentTabName);
    }

private:
    int lastNonExitTabIndex = PTStart;
};

class AppTabsHost final : public juce::Component
{
public:
    AppTabsHost()
        : playerTab(scoreTab),
          tracksTab(scoreTab)
    {
        addAndMakeVisible(tabs);
        const auto defaultTabColour = findColour(juce::ResizableWindow::backgroundColourId);
        tabs.addTab("Start", defaultTabColour, &playerTab, false);
        tabs.addTab("Score", defaultTabColour, &scoreTab, false);
        tabs.addTab("Effects", defaultTabColour, &tracksTab, false);
        tabs.addTab("Exit", defaultTabColour, &exitPlaceholder, false);

        playerTab.setExitAction([this]() { tabs.setCurrentTabIndex(PTExit, true); });

        setSize(1280, 720);
    }

    void resized() override
    {
        tabs.setBounds(getLocalBounds());
    }

    void runStartupTasks()
    {
        scoreTab.runStartupResumeIfEnabled();
    }

private:
    AppTabbedComponent tabs;
    juce::Component exitPlaceholder;
    MainComponent scoreTab;
    PlayerTabComponent playerTab;
    TracksTabComponent tracksTab;
};
