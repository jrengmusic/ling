#include "MarkdownLanguage.h"

std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> MarkdownLanguage::parse (std::string_view text, const juce::String& path) const
{
    return getParsed (jam::MarkdownDocument::parse (text, path), path);
}

std::unique_ptr<jam::Document> MarkdownLanguage::read (std::string_view text, std::string_view stored) const
{
    return jam::MarkdownDocument::read (text, stored);
}

juce::int64 MarkdownLanguage::getSymbol (const juce::File& file, std::string_view name, int role) const
{
    static constexpr const char* anchorSeparator { "#" };

    if ((role & jam::LanguageRole::reference) != 0)
    {
        const auto destination { jam::Strings::fromView (name) };
        const auto target { destination.upToFirstOccurrenceOf (anchorSeparator, false, false) };
        const auto anchor { destination.fromFirstOccurrenceOf (anchorSeparator, false, false) };

        return getSymbolHash (target.isEmpty() ? file : file.getParentDirectory().getChildFile (target), jam::Strings::getView (anchor));
    }

    return BuiltInLanguage::getSymbol (file, name, role);
}
