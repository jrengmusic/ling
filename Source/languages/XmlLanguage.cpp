#include "XmlLanguage.h"

std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> XmlLanguage::parse (std::string_view text, const juce::String& path) const
{
    return getParsed (jam::XmlDocument::parse (text), path);
}

std::unique_ptr<jam::Document> XmlLanguage::read (std::string_view text, std::string_view stored) const
{
    return jam::XmlDocument::read (text, stored);
}
