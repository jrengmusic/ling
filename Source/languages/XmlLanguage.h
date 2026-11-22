#pragma once
#include <JuceHeader.h>
#include "../BuiltInLanguage.h"

class XmlLanguage : public BuiltInLanguage<map::XmlElements, map::XmlTokenType>
{
public:
    using BuiltInLanguage<map::XmlElements, map::XmlTokenType>::BuiltInLanguage;

    std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> parse (std::string_view text, const juce::String& path) const override;
    std::unique_ptr<jam::Document> read (std::string_view text, std::string_view stored) const override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XmlLanguage)
};
