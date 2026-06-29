#pragma once

#include <optional>

namespace KeyOverrideTranspose
{
inline int tonicPcFromSignature(int sharpsOrFlats, bool isMinor)
{
    static constexpr int majorTonicPcs[15] = { 11, 6, 1, 8, 3, 10, 5, 0, 7, 2, 9, 4, 11, 6, 1 };
    static constexpr int minorTonicPcs[15] = { 8, 3, 10, 5, 0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10 };
    const int idx = sharpsOrFlats + 7;
    if (idx < 0 || idx > 14)
        return 0;
    return isMinor ? minorTonicPcs[idx] : majorTonicPcs[idx];
}

inline int normalizeSemitoneDelta(int delta)
{
    while (delta > 6)
        delta -= 12;
    while (delta < -6)
        delta += 12;
    return delta;
}

inline int appliedSemitonesAfterKeyChange(int currentApplied, int referenceTonicPc, int newOverrideTonicPc)
{
    const int delta = normalizeSemitoneDelta(newOverrideTonicPc - referenceTonicPc);
    return currentApplied + delta;
}

inline int midiTonicPc(bool hasKeySignature, int sharpsOrFlats, bool keyIsMajor)
{
    if (!hasKeySignature)
        return 0;
    // tonicPcFromSignature expects isMinor; MIDI metadata stores isMajor.
    return tonicPcFromSignature(sharpsOrFlats, !keyIsMajor);
}

inline int semitonesForKeyOverride(bool assignEnabled,
                                   bool profileOnly,
                                   std::optional<int> overrideTonicPc,
                                   bool hasKeySignature,
                                   int sharpsOrFlats,
                                   bool keyIsMajor,
                                   std::optional<int> referenceTonicPc = std::nullopt)
{
    if (assignEnabled || profileOnly || !overrideTonicPc.has_value())
        return 0;

    const int midiTonic = midiTonicPc(hasKeySignature, sharpsOrFlats, keyIsMajor);
    const int detectedTonic = referenceTonicPc.has_value() ? referenceTonicPc.value() : midiTonic;
    return normalizeSemitoneDelta(overrideTonicPc.value() - detectedTonic);
}
}
