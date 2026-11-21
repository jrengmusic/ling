/*******************************************************************************
                        Codegen Annotated Source of Truth
————————————————————————————————————————————————————————————————————————————————

            ░░████████████░░████████████░░████████████░░████████████
            ░░████  ░░████░░████  ░░████░░████  ░░████    ░░████
            ░░████        ░░████  ░░████░░████            ░░████
            ░░████        ░░████████████░░████████████    ░░████
            ░░████        ░░████  ░░████        ░░████    ░░████
            ░░████  ░░████░░████  ░░████░░████  ░░████    ░░████
            ░░████████████░░████  ░░████░░████████████    ░░████

————————————————————————————————————————————————————————————————————————————————
                         FOR YOUR EYES ONLY, DO NOT EDIT
********************************************************************************/

/**
 * @file HashMaps.h
 * @brief ling's banner rows.
 */

#pragma once

namespace map
{
/*_____________________________________________________________________________*/

extern const jam::HashMap<juce::String, juce::String> banner;

//==============================================================================

/** @brief The tree whose Id::isIndexed each operation waits for: the file row or the files child. */
struct Readiness : public jam::HashMap<juce::Identifier, juce::Identifier>
{
    Readiness() : jam::HashMap<juce::Identifier, juce::Identifier> {
    { Id::documentSymbol,       Id::file },
    { Id::hover,                Id::file },
    { Id::inlayHint,            Id::file },
    { Id::switchSourceHeader,   Id::file },
    { Id::diagnostics,          Id::file },
    { Id::publishDiagnostics,   Id::file },
    { Id::goToDefinition,       Id::dependencies },
    { Id::findReferences,       Id::file },
    { Id::workspaceSymbol,      Id::files },
    { Id::completion,           Id::file },
    { Id::goToImplementation,   Id::file },
    { Id::prepareCallHierarchy, Id::file },
    { Id::incomingCalls,        Id::file },
    { Id::outgoingCalls,        Id::file },
    { Id::didOpen,              Id::file },
    { Id::didChange,            Id::file },
    { Id::didClose,             Id::file },
    { Id::semanticTokens,       Id::file },
    { Id::semanticTokensRange,  Id::file },
    { Id::selectionRange,       Id::file },
    { Id::textObject,           Id::file },
    { Id::documentHighlight,    Id::file },
    { Id::codeAction,           Id::file },
    { Id::typeDefinition,       Id::dependencies },
    { Id::rename,               Id::file },
    } {}
};

//==============================================================================

/** @brief The argument shape of each operation: a position, a file, or a query. */
struct Parameters : public jam::HashMap<juce::Identifier, juce::Identifier>
{
    Parameters() : jam::HashMap<juce::Identifier, juce::Identifier> {
    { Id::goToDefinition,       Id::position },
    { Id::findReferences,       Id::position },
    { Id::hover,                Id::position },
    { Id::completion,           Id::position },
    { Id::goToImplementation,   Id::position },
    { Id::prepareCallHierarchy, Id::position },
    { Id::incomingCalls,        Id::position },
    { Id::outgoingCalls,        Id::position },
    { Id::selectionRange,       Id::position },
    { Id::textObject,           Id::position },
    { Id::typeDefinition,       Id::position },
    { Id::documentHighlight,    Id::position },
    { Id::rename,               Id::position },
    { Id::codeAction,           Id::range },
    { Id::documentSymbol,       Id::file },
    { Id::switchSourceHeader,   Id::file },
    { Id::inlayHint,            Id::file },
    { Id::diagnostics,          Id::file },
    { Id::publishDiagnostics,   Id::file },
    { Id::semanticTokens,       Id::file },
    { Id::semanticTokensRange,  Id::range },
    { Id::workspaceSymbol,      Id::query },
    { Id::didOpen,              Id::file },
    { Id::didChange,            Id::file },
    { Id::didClose,             Id::file },
    } {}
};

//==============================================================================

/** @brief The ling operation that answers each LSP request method. */
struct Operations : public jam::HashMap<juce::String, juce::Identifier>
{
    Operations() : jam::HashMap<juce::String, juce::Identifier> {
    { "textDocument/definition",           Id::goToDefinition },
    { "textDocument/references",           Id::findReferences },
    { "textDocument/hover",                Id::hover },
    { "textDocument/documentSymbol",       Id::documentSymbol },
    { "workspace/symbol",                  Id::workspaceSymbol },
    { "textDocument/completion",           Id::completion },
    { "textDocument/implementation",       Id::goToImplementation },
    { "textDocument/prepareCallHierarchy", Id::prepareCallHierarchy },
    { "callHierarchy/incomingCalls",       Id::incomingCalls },
    { "callHierarchy/outgoingCalls",       Id::outgoingCalls },
    { "textDocument/inlayHint",            Id::inlayHint },
    { "textDocument/semanticTokens/full",  Id::semanticTokens },
    { "textDocument/semanticTokens/range", Id::semanticTokensRange },
    { "textDocument/selectionRange",       Id::selectionRange },
    { "ling/textObject",                   Id::textObject },
    { "textDocument/declaration",          Id::goToDefinition },
    { "textDocument/typeDefinition",       Id::typeDefinition },
    { "textDocument/documentHighlight",    Id::documentHighlight },
    { "textDocument/rename",               Id::rename },
    { "textDocument/codeAction",           Id::codeAction },
    { "textDocument/switchSourceHeader",   Id::switchSourceHeader },
    } {}
};

//==============================================================================

/** @brief The extension and the name of each built-in language. */
struct Languages : public jam::HashMap<juce::String, juce::String>
{
    Languages() : jam::HashMap<juce::String, juce::String> {
    { "md",   "markdown" },
    { "mmd",  "mermaid" },
    { "json", "json" },
    { "css",  "css" },
    { "xml",  "xml" },
    { "html", "html" },
    } {}
};

//==============================================================================

/** @brief The role and the symbol type of each markdown element that holds a name. A heading declares its text. A link refers to its destination. */
struct MarkdownElements : public jam::HashMap<juce::Identifier, jam::Union<uint8_t, uint8_t>>
{
    MarkdownElements() : jam::HashMap<juce::Identifier, jam::Union<uint8_t, uint8_t>> {
    { map::HeadingTag::getInstance()->get (map::HeadingTag::h1), jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) },
    { map::HeadingTag::getInstance()->get (map::HeadingTag::h2), jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) },
    { map::HeadingTag::getInstance()->get (map::HeadingTag::h3), jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) },
    { map::HeadingTag::getInstance()->get (map::HeadingTag::h4), jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) },
    { map::HeadingTag::getInstance()->get (map::HeadingTag::h5), jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) },
    { map::HeadingTag::getInstance()->get (map::HeadingTag::h6), jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) },
    { Id::a,                                                     jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::reference, lsp::SymbolType::String) },
    } {}
};

//==============================================================================

/** @brief The role and the symbol type of each mermaid element that holds a name. A node declares its id. An edge refers to the ids of its two ends. */
struct MermaidElements : public jam::HashMap<juce::Identifier, jam::Union<uint8_t, uint8_t>>
{
    MermaidElements() : jam::HashMap<juce::Identifier, jam::Union<uint8_t, uint8_t>> {
    { Id::node, jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Object) },
    { Id::edge, jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::reference, lsp::SymbolType::Object) },
    } {}
};

//==============================================================================

/** @brief The role and the symbol type of each json element that holds a name. A key declares its text. */
struct JsonElements : public jam::HashMap<juce::Identifier, jam::Union<uint8_t, uint8_t>>
{
    JsonElements() : jam::HashMap<juce::Identifier, jam::Union<uint8_t, uint8_t>> {
    { Id::key, jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Key) },
    } {}
};

//==============================================================================

/** @brief The role and the symbol type of each name token of a css element, by the position of the token. Token 0 is the selector. */
struct CssElements : public jam::HashMap<int, jam::Union<uint8_t, uint8_t>>
{
    CssElements() : jam::HashMap<int, jam::Union<uint8_t, uint8_t>> {
    { 0, jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Class) },
    } {}
};

//==============================================================================

/** @brief The role and the symbol type of each name token of an xml element, by the position of the token. Token 0 is the tag. Token 1 is the value of the id attribute. */
struct XmlElements : public jam::HashMap<int, jam::Union<uint8_t, uint8_t>>
{
    XmlElements() : jam::HashMap<int, jam::Union<uint8_t, uint8_t>> {
    { 0, jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Object) },
    { 1, jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Constant) },
    } {}
};

//==============================================================================

/** @brief The role and the symbol type of each name token of an html element, by the position of the token. Token 0 is the tag. Token 1 is the value of the id attribute. */
struct HtmlElements : public jam::HashMap<int, jam::Union<uint8_t, uint8_t>>
{
    HtmlElements() : jam::HashMap<int, jam::Union<uint8_t, uint8_t>> {
    { 0, jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Object) },
    { 1, jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Constant) },
    } {}
};

/**______________________________END OF NAMESPACE______________________________*/
}// namespace map
