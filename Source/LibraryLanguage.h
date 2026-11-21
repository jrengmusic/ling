#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "Language.h"

class LibraryLanguage : public Language
{
public:
    explicit LibraryLanguage (std::shared_ptr<const jam::LanguageLibrary> libraryToUse);

    std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> parse (std::string_view text, const juce::String& path) const override;
    juce::Result isValid (std::string_view text) const override;
    std::unique_ptr<jam::Document> read (std::string_view text, std::string_view stored) const override;
    int index (const char* const* arguments, int argumentCount, const char* directory, const char* const* bufferPaths, const char* const* bufferTexts, int bufferCount, const jam::LanguageCallbacks& callbacks) const override;
    bool hasIndex() const override;
    int units (const char* const* arguments, const int* argumentCounts, int rowCount, const char* directory, const char* cacheDirectory, const jam::LanguageCallbacks& callbacks) const override;
    bool hasRole (const jam::Document::Element& element, int role) const override;
    int getSymbolType (const jam::Document::Element& element) const override;
    int getSemanticType (int tokenType) const override;
    int getSemanticModifiers (int tokenType) const override;
    int getCompletionType (int tokenType) const override;
    juce::StringArray getCounterparts (const juce::String& extension) const override;
    std::unique_ptr<SymbolDocument> getSymbols (const jam::Document& document, const juce::String& path) const override;

private:
    std::shared_ptr<const jam::LanguageLibrary> library;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibraryLanguage)
};
