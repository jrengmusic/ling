#pragma once
#include <JuceHeader.h>
#include "../BuiltInLanguage.h"

class MarkdownLanguage : public BuiltInLanguage<map::MarkdownElements, map::DocumentTokenType>
{
public:
    using BuiltInLanguage<map::MarkdownElements, map::DocumentTokenType>::BuiltInLanguage;

    std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> parse (std::string_view text, const juce::String& path) const override;
    std::unique_ptr<jam::Document> read (std::string_view text, std::string_view stored) const override;

protected:
    /**
     * @brief Computes the 64-bit symbol identifier for a Markdown token or link target.
     *
     * Markdown links separate a target file and an anchor with the hash character.
     * Reference tokens split @p name at that separator and pass the resolved file
     * and anchor to getSymbolHash(). An empty target keeps @p file, and a non-empty
     * target resolves from the parent directory of @p file. Non-reference tokens
     * delegate to BuiltInLanguage::getSymbol().
     *
     * @param file  Markdown file that contains the token.
     * @param name  Token text or link destination.
     * @param role  Bitmask of jam::LanguageRole flags for the token slot.
     * @return 64-bit symbol identifier for the token or link target.
     */
    juce::int64 getSymbol (const juce::File& file, std::string_view name, int role) const override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MarkdownLanguage)
};
