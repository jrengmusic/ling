#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "Language.h"

/**
 * @class LibraryLanguage
 * @brief Adapts a dynamically loaded jam::LanguageLibrary to the Language interface.
 */
class LibraryLanguage : public Language
{
public:
    /**
     * @brief Creates a backend wrapping the loaded @p libraryToUse.
     *
     * @param libraryToUse  Shared pointer to the loaded dynamic language library.
     */
    explicit LibraryLanguage (std::shared_ptr<const jam::LanguageLibrary> libraryToUse);

    /**
     * @brief Parses @p text into a jam::Document; returns a null SymbolDocument because symbols come from index().
     *
     * @param text  UTF-8 source text to parse.
     * @param path  Canonical source file path (unused by the library parser).
     * @return Pair of parsed document and nullptr.
     */
    std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> parse (std::string_view text, const juce::String& path) const override;

    /**
     * @brief Validates syntax of @p text using the loaded language library.
     *
     * @param text  UTF-8 source text to validate.
     * @return Result indicating syntax validity or an error diagnostic.
     */
    juce::Result isValid (std::string_view text) const override;

    /**
     * @brief Reconstructs a jam::Document from @p text and cached swap bytes @p stored.
     *
     * @param text    Current UTF-8 source bytes.
     * @param stored  Serialized swap file bytes.
     * @return Validated jam::Document, or nullptr when incompatible.
     */
    std::unique_ptr<jam::Document> read (std::string_view text, std::string_view stored) const override;

    /**
     * @brief Forwards unit indexing with compile arguments and unsaved buffers to the library.
     *
     * @param arguments      Compiler driver argument array.
     * @param argumentCount  Number of entries in @p arguments.
     * @param directory      Working directory for the unit.
     * @param bufferPaths    Paths of unsaved editor buffers overriding disk contents.
     * @param bufferTexts    UTF-8 texts aligned with @p bufferPaths.
     * @param bufferCount    Number of unsaved buffers in @p bufferPaths and @p bufferTexts.
     * @param callbacks      Callback table receiving symbol, outline, hover, hint, diagnostic, completion, and fix-it records.
     * @return Zero on success, or the library error code.
     */
    int index (const char* const* arguments, int argumentCount, const char* directory, const char* const* bufferPaths, const char* const* bufferTexts, int bufferCount, const jam::LanguageCallbacks& callbacks) const override;

    /**
     * @brief Reports that library languages perform unit-level indexing via index().
     *
     * @return @c true.
     */
    bool hasIndex() const override;

    /**
     * @brief Forwards translation-unit discovery from toolchain rows to the library.
     *
     * @param arguments       Flattened command argument strings across all toolchain rows.
     * @param argumentCounts  Argument count for each toolchain row.
     * @param rowCount        Number of rows in @p argumentCounts.
     * @param directory       Project root directory.
     * @param cacheDirectory  Project cache directory for build or compilation databases.
     * @param callbacks       Callbacks receiving discovered units, build files, and messages.
     * @return Zero on success, or the library error code.
     */
    int units (const char* const* arguments, const int* argumentCounts, int rowCount, const char* directory, const char* cacheDirectory, const jam::LanguageCallbacks& callbacks) const override;

    /**
     * @brief Queries the library's role flags for @p element's token type.
     *
     * @param element  Document element carrying Id::type.
     * @param role     Bitmask of jam::LanguageRole flags to test.
     * @return @c true when the element's type carries any bit of @p role.
     */
    bool hasRole (const jam::Document::Element& element, int role) const override;

    /**
     * @brief Returns the library's LSP SymbolKind integer for @p element's token type.
     *
     * @param element  Document element carrying Id::type.
     * @return LSP SymbolKind integer value.
     */
    int getSymbolType (const jam::Document::Element& element) const override;

    /**
     * @brief Maps the library's semantic token type for @p tokenType into Legend::getTypes().
     *
     * @param tokenType  Library token type integer.
     * @return Zero-based index in Legend::getTypes().
     */
    int getSemanticType (int tokenType) const override;

    /**
     * @brief Remaps the library's semantic modifier bits for @p tokenType into Legend::getModifiers().
     *
     * @param tokenType  Library token type integer.
     * @return Bitmask over Legend::getModifiers().
     */
    int getSemanticModifiers (int tokenType) const override;

    /**
     * @brief Returns the library's LSP CompletionItemKind integer for @p tokenType.
     *
     * @param tokenType  Library token type integer.
     * @return LSP CompletionItemKind integer value.
     */
    int getCompletionType (int tokenType) const override;

    /**
     * @brief Looks up paired counterpart extensions for @p extension in the library's counterparts table.
     *
     * @param extension  Source file extension without a leading dot.
     * @return Paired extensions as jam::Strings, or empty when @p extension is unpaired.
     */
    jam::Strings getCounterparts (const juce::String& extension) const override;

    /**
     * @brief Returns nullptr because library languages build SymbolDocument records during index().
     *
     * @param document  Parsed document (unused).
     * @param path      Source file path (unused).
     * @return nullptr.
     */
    std::unique_ptr<SymbolDocument> getSymbols (const jam::Document& document, const juce::String& path) const override;

private:
    std::shared_ptr<const jam::LanguageLibrary> library;///< Loaded dynamic language library backing this adapter.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibraryLanguage)
};
