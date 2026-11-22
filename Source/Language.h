#pragma once
#include <JuceHeader.h>
#include "SymbolDocument.h"

/**
 * @class Language
 * @brief Abstract language backend interface unifying built-in grammars and loaded language libraries.
 *
 * Processor and Index operate against Language polymorphically for parsing,
 * swap serialization, translation-unit discovery, unit indexing, and LSP
 * symbol classification.
 */
class Language
{
public:
    /** @brief Releases derived language backends through a base pointer. */
    virtual ~Language() = default;

    /**
     * @brief Parses @p text into a jam::Document and an optional inline SymbolDocument.
     *
     * Built-in languages build both trees synchronously; library languages
     * return a null SymbolDocument and populate symbols asynchronously in index().
     *
     * @param text  UTF-8 source text to parse.
     * @param path  Canonical source file path.
     * @return Pair of parsed document (or nullptr on failure) and optional symbol document.
     */
    virtual std::pair<std::unique_ptr<jam::Document>, std::unique_ptr<SymbolDocument>> parse (std::string_view text, const juce::String& path) const = 0;

    /**
     * @brief Validates syntax of @p text without building an index entry.
     *
     * @param text  UTF-8 source text to validate.
     * @return Result indicating syntax validity or an error diagnostic.
     */
    virtual juce::Result isValid (std::string_view text) const = 0;

    /**
     * @brief Deserializes a cached swap document against current source @p text.
     *
     * @param text    Current UTF-8 source bytes that token views reference.
     * @param stored  Serialized swap file bytes.
     * @return Validated jam::Document, or nullptr when @p stored is incompatible.
     */
    virtual std::unique_ptr<jam::Document> read (std::string_view text, std::string_view stored) const = 0;

    /**
     * @brief Runs unit-level symbol indexing for one translation unit and reports records via @p callbacks.
     *
     * @param arguments      Compiler driver argument array.
     * @param argumentCount  Number of entries in @p arguments.
     * @param directory      Working directory for the unit.
     * @param bufferPaths    Paths of unsaved editor buffers overriding disk contents.
     * @param bufferTexts    UTF-8 texts aligned with @p bufferPaths.
     * @param bufferCount    Number of unsaved buffers in @p bufferPaths and @p bufferTexts.
     * @param callbacks      Callback table receiving symbol, outline, hover, hint, diagnostic, completion, and fix-it records.
     * @return Zero on success, or a nonzero error code from the backend.
     */
    virtual int index (const char* const* arguments, int argumentCount, const char* directory, const char* const* bufferPaths, const char* const* bufferTexts, int bufferCount, const jam::LanguageCallbacks& callbacks) const = 0;

    /**
     * @brief Tells whether this backend supports unit-level indexing via index().
     *
     * @return @c true for library languages that run external index passes; @c false for built-in grammars.
     */
    virtual bool hasIndex() const = 0;

    /**
     * @brief Discovers translation units from toolchain command rows and reports them via @p callbacks.
     *
     * @param arguments       Flattened command argument strings across all toolchain rows.
     * @param argumentCounts  Argument count for each toolchain row.
     * @param rowCount        Number of rows in @p argumentCounts.
     * @param directory       Project root directory.
     * @param cacheDirectory  Project cache directory for build or compilation databases.
     * @param callbacks       Callbacks receiving discovered units, build files, and messages.
     * @return Zero on success, or a nonzero error code from the backend.
     */
    virtual int addUnits (const char* const* arguments, const int* argumentCounts, int rowCount, const char* directory, const char* cacheDirectory, const jam::LanguageCallbacks& callbacks) const = 0;

    /**
     * @brief Tells whether @p element carries the given jam::LanguageRole bitmask.
     *
     * @param element  Document element to inspect.
     * @param role     Bitmask of jam::LanguageRole flags to test.
     * @return @c true when any bit of @p role is set on @p element.
     */
    virtual bool hasRole (const jam::Document::Element& element, int role) const = 0;

    /**
     * @brief Returns the LSP SymbolKind integer for @p element.
     *
     * @param element  Document element to classify.
     * @return LSP SymbolKind integer value.
     */
    virtual int getSymbolType (const jam::Document::Element& element) const = 0;

    /**
     * @brief Maps a language-specific token type to its index in Legend::getTypes().
     *
     * @param tokenType  Language token type integer.
     * @return Zero-based semantic token type index in the shared server legend.
     */
    virtual int getSemanticType (int tokenType) const = 0;

    /**
     * @brief Maps a language-specific token type to its bitmask in Legend::getModifiers().
     *
     * @param tokenType  Language token type integer.
     * @return Bitmask of semantic token modifiers in the shared server legend.
     */
    virtual int getSemanticModifiers (int tokenType) const = 0;

    /**
     * @brief Maps a language-specific token type to an LSP CompletionItemKind integer.
     *
     * @param tokenType  Language token type integer.
     * @return LSP CompletionItemKind integer value.
     */
    virtual int getCompletionType (int tokenType) const = 0;

    /**
     * @brief Returns candidate counterpart file extensions for @p extension (without leading dot).
     *
     * @param extension  Source file extension without a leading dot.
     * @return Ordered list of paired counterpart extensions in jam::Strings, or empty when none exist.
     */
    virtual jam::Strings getCounterparts (const juce::String& extension) const = 0;

    /**
     * @brief Builds an in-memory SymbolDocument directly from a parsed @p document.
     *
     * @param document  Parsed jam::Document to extract symbols and outlines from.
     * @param path      Canonical source file path.
     * @return Built SymbolDocument, or nullptr when symbols are produced by index().
     */
    virtual std::unique_ptr<SymbolDocument> getSymbols (const jam::Document& document, const juce::String& path) const = 0;
};
