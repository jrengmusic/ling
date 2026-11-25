#include "JsonLanguage.h"

std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> JsonLanguage::parse (std::string_view text, const juce::String& path) const
{
    return getParsed (jam::JsonDocument::parse (std::string { text }), path);
}

std::unique_ptr<jam::Document> JsonLanguage::read (std::string_view text, std::string_view stored) const
{
    return jam::JsonDocument::read (text, stored);
}
