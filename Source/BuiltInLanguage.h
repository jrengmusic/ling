#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "Language.h"
#include "Legend.h"

template <typename Elements, typename TokenTypes>
class BuiltInLanguage : public Language
{
public:
    explicit BuiltInLanguage (const juce::String& extensionToUse) : extension (extensionToUse) {}

    juce::Result isValid (std::string_view) const override
    {
        return juce::Result::fail (juce::String (text::Diagnostics::failLanguageParse));
    }

    int index (const char* const*, int, const char*, const char* const*, const char* const*, int, const jam::LanguageCallbacks&) const override
    {
        return 0;
    }

    bool hasIndex() const override
    {
        return false;
    }

    int units (const char* const* arguments, const int* argumentCounts, int rowCount, const char* directory, const char*, const jam::LanguageCallbacks& callbacks) const override
    {
        int offset { 0 };

        for (int rowIndex { 0 }; rowIndex < rowCount; ++rowIndex)
        {
            const juce::Identifier command { arguments[offset] };

            if (command == Id::sourceGlob or command == Id::layoutGlob or command == Id::binary)
                addUnits (juce::File { juce::String::fromUTF8 (arguments[offset + 1]) }, directory, callbacks);

            offset += argumentCounts[rowIndex];
        }

        return 0;
    }

    juce::StringArray getCounterparts (const juce::String&) const override
    {
        return {};
    }

    std::unique_ptr<SymbolDocument> getSymbols (const jam::Document& document, const juce::String& path) const override
    {
        SymbolDocument::Builder builder;
        const auto fileIndex { builder.getOrCreateFile (jam::Strings::getView (path)) };
        const juce::File source { path };
        std::vector<const jam::Document::Element*> declarations;

        for (const auto* child : document)
            child->applyFunctionRecursively ([this, &builder, fileIndex, &document, &source, &declarations] (const jam::Document::Element& element)
            {
                addRecords (builder, fileIndex, document, element, source);

                if (hasRole (element, jam::LanguageRole::declaration))
                    declarations.push_back (&element);
            });

        addOutlines (builder, fileIndex, document, declarations);

        return builder.buildInMemory (0, 0, 0);
    }

    bool hasRole (const jam::Document::Element& element, int role) const override
    {
        const auto& elements { getElements() };
        const auto row { elements.find (getKey (element, getLastTokenIndex (element), static_cast<const typename Elements::key_type*> (nullptr))) };

        if (row != elements.end())
        {
            const auto& [key, roleType] { *row };
            return (roleType.template unpack<0>() & role) != 0;
        }

        return false;
    }

    int getSymbolType (const jam::Document::Element& element) const override
    {
        const auto& elements { getElements() };
        const auto key { getKey (element, getLastTokenIndex (element), static_cast<const typename Elements::key_type*> (nullptr)) };

        return static_cast<int> (elements.at (key).template unpack<1>());
    }

    int getSemanticType (int tokenType) const override
    {
        static const TokenTypes tokenTypes;

        return Legend::getTypeIndex (tokenTypes.get (tokenType));
    }

    int getSemanticModifiers (int) const override
    {
        return noModifiers;
    }

    int getCompletionType (int) const override
    {
        return lsp::CompletionItemType::Text;
    }

protected:
    std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> getParsed (std::unique_ptr<jam::Document> document, const juce::String& path) const
    {
        std::unique_ptr<SymbolDocument> symbols;

        if (document != nullptr)
            symbols = getSymbols (*document, path);

        return { std::move (document), std::move (symbols) };
    }

    virtual juce::int64 getSymbol (const juce::File& file, std::string_view name, int) const
    {
        return getSymbolHash (file, name);
    }

    static juce::int64 getSymbolHash (const juce::File& file, std::string_view anchor)
    {
        const auto path { file.getFullPathName() };
        const auto text { jam::Format::toValidID (jam::Strings::fromView (anchor), true) };

        return static_cast<juce::int64> (jam::Wyhash::fold (jam::Wyhash::hashBytes (path.toRawUTF8(), path.getNumBytesAsUTF8()), jam::Wyhash::hashBytes (text.toRawUTF8(), text.getNumBytesAsUTF8())));
    }

private:
    static constexpr int noModifiers { 0 };
    static constexpr int noQualifierLength { 0 };

    const juce::String extension;

    void addUnits (const juce::File& wildcard, const char* directory, const jam::LanguageCallbacks& callbacks) const
    {
        if (wildcard.getFileExtension().substring (1).compare (extension) == 0)
        {
            const auto files (wildcard.getParentDirectory().findChildFiles (juce::File::findFiles, false, wildcard.getFileName()));
            juce::StringArray paths;

            for (const auto& file : files)
                paths.add (file.getFullPathName());

            paths.sort (false);

            for (const auto& path : paths)
                callbacks.addUnit (callbacks.context, path.toRawUTF8(), directory, "", nullptr, 0, nullptr, 0, nullptr, 0);
        }
    }

    static const Elements& getElements()
    {
        static const Elements elements;
        return elements;
    }

    static juce::Identifier getKey (const jam::Document::Element& element, int, const juce::Identifier*)
    {
        return element.id;
    }

    static int getKey (const jam::Document::Element&, int tokenIndex, const int*)
    {
        return tokenIndex;
    }

    static int getLastTokenIndex (const jam::Document::Element& element)
    {
        return element.get<jam::Document::Tokens> (Id::nameTokens)->size() - 1;
    }

    void addRecords (SymbolDocument::Builder& builder, int fileIndex, const jam::Document& document, const jam::Document::Element& element, const juce::File& source) const
    {
        const auto* names { element.get<jam::Document::Tokens> (Id::nameTokens) };
        const auto& elements { getElements() };

        for (int tokenIndex { 0 }; tokenIndex < names->size(); ++tokenIndex)
        {
            if (const auto row { elements.find (getKey (element, tokenIndex, static_cast<const typename Elements::key_type*> (nullptr))) }; row != elements.end())
            {
                const auto& [key, roleType] { *row };
                const auto& token { names->at (tokenIndex) };
                const auto role { static_cast<int> (roleType.template unpack<0>()) };

                builder.addSymbol (fileIndex,
                                   getSymbol (source, document.getView (token), role),
                                   static_cast<int32_t> (document.getLineNumber (token.offset())),
                                   static_cast<int32_t> (document.getColumnNumber (token.offset())),
                                   static_cast<int32_t> (token.length()),
                                   role,
                                   static_cast<int32_t> (roleType.template unpack<1>()),
                                   jam::LanguageCallbacks::noHover,
                                   nullptr,
                                   0);
            }
        }
    }

    void addOutlines (SymbolDocument::Builder& builder, int fileIndex, const jam::Document& document, const std::vector<const jam::Document::Element*>& declarations) const
    {
        jam::HashMap<const jam::Document::Element*, int> declarationIndices;

        for (int declarationIndex { 0 }; declarationIndex < static_cast<int> (declarations.size()); ++declarationIndex)
            declarationIndices.try_emplace (declarations.at (static_cast<size_t> (declarationIndex)), declarationIndex);

        for (const auto* declaration : declarations)
            addOutline (builder, fileIndex, document, *declaration, declarationIndices);
    }

    void addOutline (SymbolDocument::Builder& builder, int fileIndex, const jam::Document& document, const jam::Document::Element& element, const jam::HashMap<const jam::Document::Element*, int>& declarationIndices) const
    {
        const auto& name { element.get<jam::Document::Tokens> (Id::nameTokens)->last() };
        const auto& range { element.get<jam::Document::Tokens> (Id::range)->at (0) };
        const auto rangeEnd { range.offset() + range.length() };

        builder.addOutline (fileIndex,
                            static_cast<int32_t> (document.getLineNumber (name.offset())),
                            static_cast<int32_t> (document.getColumnNumber (name.offset())),
                            static_cast<int32_t> (document.getLineNumber (rangeEnd)),
                            static_cast<int32_t> (document.getColumnNumber (rangeEnd)),
                            static_cast<int32_t> (name.length()),
                            noQualifierLength,
                            getSymbolType (element),
                            getParentIndex (element, declarationIndices));
    }

    static int getParentIndex (const jam::Document::Element& element, const jam::HashMap<const jam::Document::Element*, int>& declarationIndices)
    {
        for (const auto* ancestor { element.parent }; ancestor != nullptr and ancestor->parent != nullptr; ancestor = ancestor->parent)
        {
            if (const auto entry { declarationIndices.find (ancestor) }; entry != declarationIndices.end())
            {
                const auto& [declaration, declarationIndex] { *entry };

                return declarationIndex;
            }
        }

        return SymbolDocument::noParent;
    }
};
