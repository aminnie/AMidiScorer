#pragma once

#include <JuceHeader.h>

inline juce::File getTestFixturesDirectory()
{
#if defined(MIDISCORER_TEST_FIXTURES_DIR)
    return juce::File(MIDISCORER_TEST_FIXTURES_DIR);
#else
    const juce::File testFilePath(__FILE__);
    return testFilePath.getParentDirectory().getChildFile("fixtures");
#endif
}

inline juce::File getTestFixtureFile(const juce::String& filename)
{
    return getTestFixturesDirectory().getChildFile(filename);
}
