#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "Language.h"
#include "Legend.h"

/**
 * @class BuiltInLanguage
 * @brief Template base implementing Language for built-in document grammars.
 *
 * Maps element identifiers or name-token positions through @p Elements to emit
 * in-memory SymbolDocument records and outlines directly from parsed ASTs, and
 * maps token types through @p TokenTypes to Legend::getTypes().
 *
 * @tparam Elements    Lookup table mapping element ID or token index to packed (role, symbolType).
 * @tparam TokenTypes  Bimap mapping grammar token types to semantic token type names.
 */
template <typename Elements, typename TokenTypes>
class BuiltInLanguage : public Language
{
public:
    /**
     * @brief Creates a built-in language backend matching files with @p extensionToUse.
     *
     * @param extensionToUse  File extension without a leading dot (such as "md" or "html").
     */
    explicit BuiltInLanguage (const juce::String& extensionToUse) : extension (extensionToUse) {}

    /**
     * @brief Returns a parse failure result because built-in languages validate during parse().
     *
     * @return Failed juce::Result carrying text::Diagnostics::failLanguageParse.
     */
    juce::Result isValid (std::string_view) const override
    {
        return juce::Result::fail (juce::String (text::Diagnostics::failLanguageParse));
    }

    /**
     * @brief No-op because built-in languages build symbols inline from parse() rather than an index pass.
     *
     * @return Zero.
     */
    int index (const char* const*, int, const char*, const char* const*, const char* const*, int, const jam::LanguageCallbacks&) const override
    {
        return 0;
    }

    /**
     * @brief Reports that built-in languages have no external unit-level index pass.
     *
     * @return @c false.
     */
    bool hasIndex() const override
    {
        return false;
    }

    /**
     * @brief Discovers matching source files from sourceGlob, layoutGlob, and binary toolchain rows.
     *
     * @param arguments       Flattened command argument strings across all toolchain rows.
     * @param argumentCounts  Argument count for each toolchain row.
     * @param rowCount        Number of rows in @p argumentCounts.
     * @param directory       Project root directory passed to each discovered unit.
     * @param callbacks       Callbacks receiving each discovered file via addUnit.
     * @return Zero.
     */
    int addUnits (const char* const* arguments, const int* argumentCounts, int rowCount, const char* directory, const char*, const jam::LanguageCallbacks& callbacks) const override
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

    /**
     * @brief Returns an empty jam::Strings list because built-in languages have no counterpart extensions.
     *
     * @return Empty jam::Strings.
     */
    jam::Strings getCounterparts (const juce::String&) const override
    {
        return {};
    }

    /**
     * @brief Builds an in-memory SymbolDocument from @p document by collecting records and outlines.
     *
     * Walks the AST into a SymbolDocument::Builder, appends declaration elements
     * into a jam::Array, and constructs nested outline entries with parent links.
     *
     * @param document  Parsed jam::Document to index.
     * @param path      Canonical source file path.
     * @return In-memory SymbolDocument holding all emitted symbol and outline entries.
     */
    std::unique_ptr<SymbolDocument> getSymbols (const jam::Document& document, const juce::String& path) const override
    {
        SymbolDocument::Builder builder;
        const auto fileIndex { builder.getOrCreateFile (jam::Strings::getView (path)) };
        const juce::File source { path };
        jam::Array<const jam::Document::Element*> declarations;

        for (const auto* child : document)
            child->applyFunctionRecursively ([this, &builder, fileIndex, &document, &source, &declarations] (const jam::Document::Element& element)
            {
                addRecords (builder, fileIndex, document, element, source);

                if (hasRole (element, jam::LanguageRole::declaration))
                    declarations.add (&element);
            });

        addOutlines (builder, fileIndex, document, declarations);

        return builder.buildInMemory (0, 0, 0);
    }

    /**
     * @brief Tells whether @p element carries @p role according to the Elements table.
     *
     * @param element  Document element to inspect.
     * @param role     Bitmask of jam::LanguageRole flags to test.
     * @return @c true when the element's packed role mask intersects @p role.
     */
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

    /**
     * @brief Returns the LSP SymbolKind integer packed for @p element in Elements.
     *
     * @param element  Document element to classify.
     * @return LSP SymbolKind integer value.
     */
    int getSymbolType (const jam::Document::Element& element) const override
    {
        const auto& elements { getElements() };
        const auto key { getKey (element, getLastTokenIndex (element), static_cast<const typename Elements::key_type*> (nullptr)) };

        return static_cast<int> (elements.at (key).template unpack<1>());
    }

    /**
     * @brief Maps a grammar @p tokenType via TokenTypes to its index in Legend::getTypes().
     *
     * @param tokenType  Grammar token type integer.
     * @return Zero-based semantic token type index.
     */
    int getSemanticType (int tokenType) const override
    {
        static const TokenTypes tokenTypes;

        return Legend::getTypeIndex (tokenTypes.get (tokenType));
    }

    /**
     * @brief Returns zero because built-in grammars define no semantic token modifiers.
     *
     * @return noModifiers.
     */
    int getSemanticModifiers (int) const override
    {
        return noModifiers;
    }

    /**
     * @brief Returns the default Text completion item kind for built-in grammars.
     *
     * @return lsp::CompletionItemType::Text.
     */
    int getCompletionType (int) const override
    {
        return lsp::CompletionItemType::Text;
    }

protected:
    /**
     * @brief Pairs a parsed @p document with its SymbolDocument when @p document is non-null.
     *
     * @param document  Parsed jam::Document, or nullptr on parse failure.
     * @param path      Canonical source file path.
     * @return Pair of @p document and its generated SymbolDocument (or nullptr).
     */
    std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> getParsed (std::unique_ptr<jam::Document> document, const juce::String& path) const
    {
        std::unique_ptr<SymbolDocument> symbols;

        if (document != nullptr)
            symbols = getSymbols (*document, path);

        return { std::move (document), std::move (symbols) };
    }

    /**
     * @brief Computes the symbol identifier for an element token; subclasses override for link targets.
     *
     * @param file  Source file containing the element.
     * @param name  Token text of the element name or link target.
     * @return 64-bit symbol identifier.
     */
    virtual juce::int64 getSymbol (const juce::File& file, std::string_view name, int) const
    {
        return getSymbolHash (file, name);
    }

    /**
     * @brief Hashes @p file path and normalized @p anchor into a 64-bit symbol identifier.
     *
     * @param file    Target source file.
     * @param anchor  Anchor or identifier text normalized with jam::Format::toValidID.
     * @return 64-bit folded Wyhash symbol identifier.
     */
    static juce::int64 getSymbolHash (const juce::File& file, std::string_view anchor)
    {
        const auto path { file.getFullPathName() };
        const auto text { jam::Format::toValidID (jam::Strings::fromView (anchor), true) };

        return static_cast<juce::int64> (jam::Wyhash::fold (jam::Wyhash::hashBytes (path.toRawUTF8(), path.getNumBytesAsUTF8()), jam::Wyhash::hashBytes (text.toRawUTF8(), text.getNumBytesAsUTF8())));
    }

private:
    static constexpr int noModifiers { 0 };///< Semantic modifier bitmask for built-in grammars.
    static constexpr int noQualifierLength { 0 };///< Qualifier prefix byte length for built-in outlines.

    const juce::String extension;///< Lowercase file extension (without leading dot) handled by this instance.

    /**
     * @brief Collects files matching @p wildcard into sorted jam::Strings and registers each unit.
     *
     * @param wildcard   File wildcard path from a pattern toolchain row.
     * @param directory  Project root directory passed to callbacks.addUnit.
     * @param callbacks  Callback table receiving each sorted unit path.
     */
    void addUnits (const juce::File& wildcard, const char* directory, const jam::LanguageCallbacks& callbacks) const
    {
        if (wildcard.getFileExtension().substring (1).compare (extension) == 0)
        {
            const auto files (wildcard.getParentDirectory().findChildFiles (juce::File::findFiles, false, wildcard.getFileName()));
            jam::Strings paths;

            for (const auto& file : files)
                paths.add (file.getFullPathName());

            paths.sort (false);

            for (const auto& path : paths)
                callbacks.addUnit (callbacks.context, path.toRawUTF8(), directory, "", nullptr, 0, nullptr, 0, nullptr, 0);
        }
    }

    /**
     * @brief Returns the static Elements table mapping keys to (role, symbolType) pairs.
     *
     * @return Const reference to the Elements table.
     */
    static const Elements& getElements()
    {
        static const Elements elements;
        return elements;
    }

    /**
     * @brief Extracts the Elements key when keyed by element identifier.
     *
     * @param element  Document element whose id is the key.
     * @return Element identifier key.
     */
    static juce::Identifier getKey (const jam::Document::Element& element, int, const juce::Identifier*)
    {
        return element.id;
    }

    /**
     * @brief Extracts the Elements key when keyed by name-token slot index.
     *
     * @param tokenIndex  Zero-based index in Id::nameTokens.
     * @return Token slot index key.
     */
    static int getKey (const jam::Document::Element&, int tokenIndex, const int*)
    {
        return tokenIndex;
    }

    /**
     * @brief Returns the index of the last token in @p element's Id::nameTokens array.
     *
     * @param element  Document element carrying Id::nameTokens.
     * @return Last token index.
     */
    static int getLastTokenIndex (const jam::Document::Element& element)
    {
        return element.get<jam::Document::Tokens> (Id::nameTokens)->size() - 1;
    }

    /**
     * @brief Emits symbol records for each non-empty name token of @p element present in Elements.
     *
     * Skips zero-length placeholder tokens so absent optional slots (such as missing
     * attributes in positional name-token lists) emit no empty symbol records.
     *
     * @param builder    SymbolDocument builder receiving the symbol records.
     * @param fileIndex  File index in @p builder for @p source.
     * @param document   Parsed jam::Document owning @p element.
     * @param element    Element whose name tokens are inspected.
     * @param source     Canonical source file for symbol hashing.
     */
    void addRecords (SymbolDocument::Builder& builder, int fileIndex, const jam::Document& document, const jam::Document::Element& element, const juce::File& source) const
    {
        const auto* names { element.get<jam::Document::Tokens> (Id::nameTokens) };
        const auto& elements { getElements() };

        for (int tokenIndex { 0 }; tokenIndex < names->size(); ++tokenIndex)
        {
            const auto& token { names->at (tokenIndex) };

            if (token.length() > 0)
            {
                if (const auto row { elements.find (getKey (element, tokenIndex, static_cast<const typename Elements::key_type*> (nullptr))) }; row != elements.end())
                {
                    const auto& [key, roleType] { *row };
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
    }

    /**
     * @brief Builds an index map for @p declarations and appends an outline entry for each declaration.
     *
     * @param builder       SymbolDocument builder receiving the outline entries.
     * @param fileIndex     File index in @p builder.
     * @param document      Parsed jam::Document owning @p declarations.
     * @param declarations  Ordered array of declaration elements in @p document.
     */
    void addOutlines (SymbolDocument::Builder& builder, int fileIndex, const jam::Document& document, const jam::Array<const jam::Document::Element*>& declarations) const
    {
        jam::HashMap<const jam::Document::Element*, int> declarationIndices;

        for (int declarationIndex { 0 }; declarationIndex < declarations.size(); ++declarationIndex)
            declarationIndices.try_emplace (declarations.at (declarationIndex), declarationIndex);

        for (const auto* declaration : declarations)
            addOutline (builder, fileIndex, document, *declaration, declarationIndices);
    }

    /**
     * @brief Appends one outline entry for @p element using its last name token and full range.
     *
     * @param builder             SymbolDocument builder receiving the outline entry.
     * @param fileIndex           File index in @p builder.
     * @param document            Parsed jam::Document owning @p element.
     * @param element             Declaration element to emit.
     * @param declarationIndices  Map from declaration element pointer to its outline slot index.
     */
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

    /**
     * @brief Finds the outline slot index of @p element's nearest enclosing declaration ancestor.
     *
     * @param element             Declaration element whose parent is sought.
     * @param declarationIndices  Map from declaration element pointer to its outline slot index.
     * @return Enclosing declaration index, or SymbolDocument::noParent when top-level.
     */
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
