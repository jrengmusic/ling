#pragma once
#include <JuceHeader.h>
#include "SymbolDocument.h"

class Language
{
public:
    virtual ~Language() = default;

    virtual std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> parse (std::string_view text, const juce::String& path) const = 0;
    virtual juce::Result isValid (std::string_view text) const = 0;
    virtual std::unique_ptr<jam::Document> read (std::string_view text, std::string_view stored) const = 0;
    virtual int index (const char* const* arguments, int argumentCount, const char* directory, const char* const* bufferPaths, const char* const* bufferTexts, int bufferCount, const jam::LanguageCallbacks& callbacks) const = 0;
    virtual bool hasIndex() const = 0;
    virtual int units (const char* const* arguments, const int* argumentCounts, int rowCount, const char* directory, const char* cacheDirectory, const jam::LanguageCallbacks& callbacks) const = 0;
    virtual bool hasRole (const jam::Document::Element& element, int role) const = 0;
    virtual int getSymbolType (const jam::Document::Element& element) const = 0;
    virtual int getSemanticType (int tokenType) const = 0;
    virtual int getSemanticModifiers (int tokenType) const = 0;
    virtual int getCompletionType (int tokenType) const = 0;
    virtual juce::StringArray getCounterparts (const juce::String& extension) const = 0;
    virtual std::unique_ptr<SymbolDocument> getSymbols (const jam::Document& document, const juce::String& path) const = 0;
};
