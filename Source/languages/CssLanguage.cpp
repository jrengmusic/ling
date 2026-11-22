#include "CssLanguage.h"

std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> CssLanguage::parse (std::string_view text, const juce::String& path) const
{
    return getParsed (jam::CssDocument::parse (text), path);
}

std::unique_ptr<jam::Document> CssLanguage::read (std::string_view text, std::string_view stored) const
{
    return jam::CssDocument::read (text, stored);
}
