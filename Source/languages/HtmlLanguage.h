#pragma once
#include <JuceHeader.h>
#include "../BuiltInLanguage.h"

/**
 * @class HtmlLanguage
 * @brief BuiltInLanguage specialization for HTML documents using map::HtmlElements and map::HtmlTokenType.
 */
class HtmlLanguage : public BuiltInLanguage<map::HtmlElements, map::HtmlTokenType>
{
public:
    using BuiltInLanguage<map::HtmlElements, map::HtmlTokenType>::BuiltInLanguage;

    /**
     * @brief Parses HTML @p text and builds its inline SymbolDocument via getParsed().
     *
     * @param text  UTF-8 HTML source bytes.
     * @param path  Canonical source file path.
     * @return Pair of parsed jam::Document and its built SymbolDocument.
     */
    std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> parse (std::string_view text, const juce::String& path) const override;

    /**
     * @brief Reconstructs a cached HTML jam::Document from @p text and swap bytes @p stored.
     *
     * @param text    Current UTF-8 source bytes.
     * @param stored  Serialized swap file bytes.
     * @return Deserialized jam::Document, or nullptr if incompatible.
     */
    std::unique_ptr<jam::Document> read (std::string_view text, std::string_view stored) const override;

protected:
    /**
     * @brief Resolves `#id` anchor references to the target file and normalized fragment identifier.
     *
     * For tokens carrying jam::LanguageRole::reference (such as `href` values), splits
     * @p name at `#`: the substring before `#` resolves relative to @p file's parent directory
     * (or @p file itself when empty for same-file `#id` anchors), and the substring after `#`
     * is hashed via getSymbolHash(). Declarations fall back to BuiltInLanguage::getSymbol().
     *
     * @param file  HTML file containing the token.
     * @param name  Token text (tag name, `id` value, or `href` destination).
     * @param role  Bitmask of jam::LanguageRole flags for the token slot.
     * @return 64-bit symbol identifier matching the target declaration.
     */
    juce::int64 getSymbol (const juce::File& file, std::string_view name, int role) const override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HtmlLanguage)
};
