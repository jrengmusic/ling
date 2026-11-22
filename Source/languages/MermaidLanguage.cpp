#include "MermaidLanguage.h"

std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> MermaidLanguage::parse (std::string_view text, const juce::String& path) const
{
    return getParsed (jam::MermaidDiagram::parse (text), path);
}

std::unique_ptr<jam::Document> MermaidLanguage::read (std::string_view text, std::string_view stored) const
{
    return jam::MermaidDiagram::read (text, stored);
}
