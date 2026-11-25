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

#include <JuceHeader.h>
#include "Generated.h"

namespace Id
{
/*_____________________________________________________________________________*/

/** @brief LING vocabulary — the words of the ling command line and of the state tree. */

    const juce::Identifier addSymbols           { juce::String::fromUTF8 ("addSymbols") };
    const juce::Identifier argument             { juce::String::fromUTF8 ("argument") };
    const juce::Identifier arguments            { juce::String::fromUTF8 ("arguments") };
    const juce::Identifier binary               { juce::String::fromUTF8 ("binary") };
    const juce::Identifier buffer               { juce::String::fromUTF8 ("buffer") };
    const juce::Identifier buffers              { juce::String::fromUTF8 ("buffers") };
    const juce::Identifier build                { juce::String::fromUTF8 ("build") };
    const juce::Identifier buildFiles           { juce::String::fromUTF8 ("buildFiles") };
    const juce::Identifier cancel               { juce::String::fromUTF8 ("cancel") };
    const juce::Identifier cast                 { juce::String::fromUTF8 ("cast") };
    const juce::Identifier children             { juce::String::fromUTF8 ("children") };
    const juce::Identifier cmake                { juce::String::fromUTF8 ("cmake") };
    const juce::Identifier codeAction           { juce::String::fromUTF8 ("codeAction") };
    const juce::Identifier column               { juce::String::fromUTF8 ("column") };
    const juce::Identifier completion           { juce::String::fromUTF8 ("completion") };
    const juce::Identifier completions          { juce::String::fromUTF8 ("completions") };
    const juce::Identifier connection           { juce::String::fromUTF8 ("connection") };
    const juce::Identifier connections          { juce::String::fromUTF8 ("connections") };
    const juce::Identifier daemon               { juce::String::fromUTF8 ("daemon") };
    const juce::Identifier declarations         { juce::String::fromUTF8 ("declarations") };
    const juce::Identifier dependencies         { juce::String::fromUTF8 ("dependencies") };
    const juce::Identifier diagnostics          { juce::String::fromUTF8 ("diagnostics") };
    const juce::Identifier documentHighlight    { juce::String::fromUTF8 ("documentHighlight") };
    const juce::Identifier documentSymbol       { juce::String::fromUTF8 ("documentSymbol") };
    const juce::Identifier documents            { juce::String::fromUTF8 ("documents") };
    const juce::Identifier endColumn            { juce::String::fromUTF8 ("endColumn") };
    const juce::Identifier extension            { juce::String::fromUTF8 ("extension") };
    const juce::Identifier extensions           { juce::String::fromUTF8 ("extensions") };
    const juce::Identifier family               { juce::String::fromUTF8 ("family") };
    const juce::Identifier files                { juce::String::fromUTF8 ("files") };
    const juce::Identifier findReferences       { juce::String::fromUTF8 ("findReferences") };
    const juce::Identifier fixIts               { juce::String::fromUTF8 ("fixIts") };
    const juce::Identifier flag                 { juce::String::fromUTF8 ("flag") };
    const juce::Identifier getFiles             { juce::String::fromUTF8 ("getFiles") };
    const juce::Identifier goToDefinition       { juce::String::fromUTF8 ("goToDefinition") };
    const juce::Identifier goToImplementation   { juce::String::fromUTF8 ("goToImplementation") };
    const juce::Identifier hash                 { juce::String::fromUTF8 ("hash") };
    const juce::Identifier help                 { juce::String::fromUTF8 ("help") };
    const juce::Identifier hover                { juce::String::fromUTF8 ("hover") };
    const juce::Identifier hovers               { juce::String::fromUTF8 ("hovers") };
    const juce::Identifier incomingCalls        { juce::String::fromUTF8 ("incomingCalls") };
    const juce::Identifier initialized          { juce::String::fromUTF8 ("initialized") };
    const juce::Identifier inlayHint            { juce::String::fromUTF8 ("inlayHint") };
    const juce::Identifier inlayHints           { juce::String::fromUTF8 ("inlayHints") };
    const juce::Identifier isIndexed            { juce::String::fromUTF8 ("isIndexed") };
    const juce::Identifier isInitialized        { juce::String::fromUTF8 ("isInitialized") };
    const juce::Identifier isRequested          { juce::String::fromUTF8 ("isRequested") };
    const juce::Identifier isShutDown           { juce::String::fromUTF8 ("isShutDown") };
    const juce::Identifier jobCount             { juce::String::fromUTF8 ("jobCount") };
    const juce::Identifier jucePath             { juce::String::fromUTF8 ("jucePath") };
    const juce::Identifier juceVersion          { juce::String::fromUTF8 ("juceVersion") };
    const juce::Identifier language             { juce::String::fromUTF8 ("language") };
    const juce::Identifier layoutGlob           { juce::String::fromUTF8 ("layout_glob") };
    const juce::Identifier ling                 { juce::String::fromUTF8 ("ling") };
    const juce::Identifier mcp                  { juce::String::fromUTF8 ("mcp") };
    const juce::Identifier module               { juce::String::fromUTF8 ("module") };
    const juce::Identifier nameLength           { juce::String::fromUTF8 ("nameLength") };
    const juce::Identifier newName              { juce::String::fromUTF8 ("newName") };
    const juce::Identifier outgoingCalls        { juce::String::fromUTF8 ("outgoingCalls") };
    const juce::Identifier outline              { juce::String::fromUTF8 ("outline") };
    const juce::Identifier output               { juce::String::fromUTF8 ("output") };
    const juce::Identifier paddingLeft          { juce::String::fromUTF8 ("paddingLeft") };
    const juce::Identifier paddingRight         { juce::String::fromUTF8 ("paddingRight") };
    const juce::Identifier patch                { juce::String::fromUTF8 ("patch") };
    const juce::Identifier pendingJobCount      { juce::String::fromUTF8 ("pendingJobCount") };
    const juce::Identifier port                 { juce::String::fromUTF8 ("port") };
    const juce::Identifier positionEncoding     { juce::String::fromUTF8 ("positionEncoding") };
    const juce::Identifier prepareCallHierarchy { juce::String::fromUTF8 ("prepareCallHierarchy") };
    const juce::Identifier prerequisites        { juce::String::fromUTF8 ("prerequisites") };
    const juce::Identifier profiler             { juce::String::fromUTF8 ("profiler") };
    const juce::Identifier publishDiagnostics   { juce::String::fromUTF8 ("publishDiagnostics") };
    const juce::Identifier qualifierLength      { juce::String::fromUTF8 ("qualifierLength") };
    const juce::Identifier query                { juce::String::fromUTF8 ("query") };
    const juce::Identifier references           { juce::String::fromUTF8 ("references") };
    const juce::Identifier removeBuffer         { juce::String::fromUTF8 ("removeBuffer") };
    const juce::Identifier rename               { juce::String::fromUTF8 ("rename") };
    const juce::Identifier request              { juce::String::fromUTF8 ("request") };
    const juce::Identifier result               { juce::String::fromUTF8 ("result") };
    const juce::Identifier role                 { juce::String::fromUTF8 ("role") };
    const juce::Identifier roots                { juce::String::fromUTF8 ("roots") };
    const juce::Identifier runIndexJob          { juce::String::fromUTF8 ("runIndexJob") };
    const juce::Identifier runParseJob          { juce::String::fromUTF8 ("runParseJob") };
    const juce::Identifier schema               { juce::String::fromUTF8 ("schema") };
    const juce::Identifier scope                { juce::String::fromUTF8 ("scope") };
    const juce::Identifier selectionRange       { juce::String::fromUTF8 ("selectionRange") };
    const juce::Identifier semanticTokens       { juce::String::fromUTF8 ("semanticTokens") };
    const juce::Identifier semanticTokensRange  { juce::String::fromUTF8 ("semanticTokensRange") };
    const juce::Identifier setBuffer            { juce::String::fromUTF8 ("setBuffer") };
    const juce::Identifier setDocuments         { juce::String::fromUTF8 ("setDocuments") };
    const juce::Identifier severity             { juce::String::fromUTF8 ("severity") };
    const juce::Identifier snippet              { juce::String::fromUTF8 ("snippet") };
    const juce::Identifier sourceGlob           { juce::String::fromUTF8 ("source_glob") };
    const juce::Identifier stdioClient          { juce::String::fromUTF8 ("stdio_client") };
    const juce::Identifier streams              { juce::String::fromUTF8 ("streams") };
    const juce::Identifier submodule            { juce::String::fromUTF8 ("submodule") };
    const juce::Identifier suite                { juce::String::fromUTF8 ("suite") };
    const juce::Identifier swap                 { juce::String::fromUTF8 ("swap") };
    const juce::Identifier switchSourceHeader   { juce::String::fromUTF8 ("switchSourceHeader") };
    const juce::Identifier symbol               { juce::String::fromUTF8 ("symbol") };
    const juce::Identifier symbols              { juce::String::fromUTF8 ("symbols") };
    const juce::Identifier textObject           { juce::String::fromUTF8 ("textObject") };
    const juce::Identifier threads              { juce::String::fromUTF8 ("threads") };
    const juce::Identifier timeout              { juce::String::fromUTF8 ("timeout") };
    const juce::Identifier toolchain            { juce::String::fromUTF8 ("toolchain") };
    const juce::Identifier toolchainArgument    { juce::String::fromUTF8 ("toolchainArgument") };
    const juce::Identifier typeDefinition       { juce::String::fromUTF8 ("typeDefinition") };
    const juce::Identifier typeSymbols          { juce::String::fromUTF8 ("typeSymbols") };
    const juce::Identifier userModule           { juce::String::fromUTF8 ("@user-module") };
    const juce::Identifier workspaceSymbol      { juce::String::fromUTF8 ("workspaceSymbol") };

/**______________________________END OF NAMESPACE______________________________*/
}// namespace Id
namespace files
{
/*_____________________________________________________________________________*/

/** @brief File, folder, and variable names that ling reads or writes, such as the help and the language files. */

const juce::String archiveExtension           { juce::String::fromUTF8 ("zip") };
const juce::String castJuceCache              { juce::String::fromUTF8 (".cache/cast/juce") };
const juce::String castJuceLocalCache         { juce::String::fromUTF8 ("cast/juce") };
const juce::String core                       { juce::String::fromUTF8 ("core.md") };
const juce::String defaultLanguage            { juce::String::fromUTF8 ("cLang") };
const juce::String language                   { juce::String::fromUTF8 ("language") };
const juce::String languageConstruction       { juce::String::fromUTF8 ("construction.md") };
const juce::String languageCore               { juce::String::fromUTF8 ("core") };
const juce::String languageCoreHeader         { juce::String::fromUTF8 ("ling_LanguageCore.h") };
const juce::String languageCoreLibrary        { juce::String::fromUTF8 ("build/liblanguageCore.a") };
const juce::String languageCoreLibraryWindows { juce::String::fromUTF8 ("build/languageCore.lib") };
const juce::String languageInterface          { juce::String::fromUTF8 ("jam_LanguageInterface.h") };
const juce::String languageLibrary            { juce::String::fromUTF8 ("build/liblanguage.dylib") };
const juce::String languageLibraryWindows     { juce::String::fromUTF8 ("build/language.dll") };
const juce::String languageSpell              { juce::String::fromUTF8 ("spell.md") };
const juce::String languageTemplate           { juce::String::fromUTF8 ("language.cast") };
const juce::String lingCache                  { juce::String::fromUTF8 (".cache/ling") };
const juce::String lingConfig                 { juce::String::fromUTF8 (".config/ling") };
const juce::String lingHelp                   { juce::String::fromUTF8 ("HELP.frame") };
const juce::String lingLocalCache             { juce::String::fromUTF8 ("ling") };
const juce::String lingPort                   { juce::String::fromUTF8 ("port") };
const juce::String lingSwap                   { juce::String::fromUTF8 ("swap") };
const juce::String lingSymbols                { juce::String::fromUTF8 ("symbols") };
const juce::String lingTools                  { juce::String::fromUTF8 ("tools.json") };
const juce::String lsp                        { juce::String::fromUTF8 ("lsp.md") };
const juce::String screenErase                { juce::String::fromUTF8 ("erase.frame") };
const juce::String sourceDirectoryVariable    { juce::String::fromUTF8 ("${CMAKE_CURRENT_SOURCE_DIR}") };
const juce::String syntaxTree                 { juce::String::fromUTF8 ("syntax_tree.md") };
const juce::String userModulePathVariable     { juce::String::fromUTF8 ("${CAST_USER_MODULE_PATH}") };
const juce::String userModulesInfo            { juce::String::fromUTF8 ("user-modules-info.md") };

/**______________________________END OF NAMESPACE______________________________*/
}// namespace files
namespace map
{
/*_____________________________________________________________________________*/

const jam::HashMap<juce::String, juce::String> banner
{
    { juce::String::fromUTF8 ("creamyMint"),     juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88                            ") },
    { juce::String::fromUTF8 ("grapeGreen"),     juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91                            ") },
    { juce::String::fromUTF8 ("sweetMintPesto"), juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88      \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("leek"),           juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("snarkyMint"),     juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("herbivore"),      juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("jovialJade"),     juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("basilSmash"),     juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("lima"),           juce::String::fromUTF8 ("\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x91\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("lightMint"),      juce::String::fromUTF8 ("\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91  \xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91  \xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91    \xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91    \xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("bigSpender"),     juce::String::fromUTF8 ("                                  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88") },
    { juce::String::fromUTF8 ("pastelPea"),      juce::String::fromUTF8 ("                            \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x91\xe2\x96\x91") },
    { juce::String::fromUTF8 ("feijoa"),         juce::String::fromUTF8 ("                            \xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91\xe2\x96\x91") },
};

/**______________________________END OF NAMESPACE______________________________*/
}// namespace map
