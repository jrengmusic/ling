#include "LibraryLanguage.h"
#include "Legend.h"

LibraryLanguage::LibraryLanguage (std::shared_ptr<const jam::LanguageLibrary> libraryToUse)
    : library (std::move (libraryToUse))
{
}

std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> LibraryLanguage::parse (std::string_view text, const juce::String&) const
{
    return { jam::LanguageDocument::parse (text, library), nullptr };
}

juce::Result LibraryLanguage::isValid (std::string_view text) const
{
    return jam::LanguageDocument::isValid (text, library);
}

std::unique_ptr<jam::Document> LibraryLanguage::read (std::string_view text, std::string_view stored) const
{
    return jam::LanguageDocument::read (text, stored, library);
}

int LibraryLanguage::index (const char* const* arguments, int argumentCount, const char* directory, const char* const* bufferPaths, const char* const* bufferTexts, int bufferCount, const jam::LanguageCallbacks& callbacks) const
{
    return library->index (arguments, argumentCount, directory, bufferPaths, bufferTexts, bufferCount, callbacks);
}

bool LibraryLanguage::hasIndex() const
{
    return true;
}

int LibraryLanguage::units (const char* const* arguments, const int* argumentCounts, int rowCount, const char* directory, const char* cacheDirectory, const jam::LanguageCallbacks& callbacks) const
{
    return library->units (arguments, argumentCounts, rowCount, directory, cacheDirectory, callbacks);
}

bool LibraryLanguage::hasRole (const jam::Document::Element& element, int role) const
{
    return library->hasRole (*element.get<int> (Id::type), role);
}

int LibraryLanguage::getSymbolType (const jam::Document::Element& element) const
{
    return library->getSymbolType (*element.get<int> (Id::type));
}

int LibraryLanguage::getSemanticType (int tokenType) const
{
    return Legend::getTypeIndex (library->semanticTokenTypes.at (library->getSemanticType (tokenType)));
}

int LibraryLanguage::getSemanticModifiers (int tokenType) const
{
    const auto libraryModifiers { library->getSemanticModifiers (tokenType) };
    int serverModifiers { 0 };

    for (int modifierIndex { 0 }; modifierIndex < library->semanticTokenModifiers.size(); ++modifierIndex)
        if ((libraryModifiers & (1 << modifierIndex)) != 0)
            serverModifiers |= Legend::getModifierBit (library->semanticTokenModifiers.at (modifierIndex));

    return serverModifiers;
}

int LibraryLanguage::getCompletionType (int tokenType) const
{
    return library->getCompletionType (tokenType);
}

juce::StringArray LibraryLanguage::getCounterparts (const juce::String& extension) const
{
    if (const auto pairing { library->counterparts.find (extension) }; pairing != library->counterparts.end())
    {
        const auto& [pairedExtension, extensions] { *pairing };
        return extensions;
    }

    return {};
}

std::unique_ptr<SymbolDocument> LibraryLanguage::getSymbols (const jam::Document&, const juce::String&) const
{
    return nullptr;
}
