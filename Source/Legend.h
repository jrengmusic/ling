#pragma once
#include <JuceHeader.h>

class Legend
{
public:
    static const jam::Strings& getTypes();
    static const jam::Strings& getModifiers();
    static int getTypeIndex (juce::StringRef name);
    static int getModifierBit (juce::StringRef name);

private:
    static jam::Strings buildTypes();
    static jam::Strings buildModifiers();
    static void addNames (jam::Strings& names, const jam::Bimap<int>& tokenNames);
};
