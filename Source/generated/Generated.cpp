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

    const juce::Identifier argument                { juce::String::fromUTF8 ("argument") };
    const juce::Identifier arguments               { juce::String::fromUTF8 ("arguments") };
    const juce::Identifier cast                    { juce::String::fromUTF8 ("cast") };
    const juce::Identifier checkFlags              { juce::String::fromUTF8 ("checkFlags") };
    const juce::Identifier checkFlagsMac           { juce::String::fromUTF8 ("checkFlags mac") };
    const juce::Identifier checkFlagsWindows       { juce::String::fromUTF8 ("checkFlags windows") };
    const juce::Identifier children                { juce::String::fromUTF8 ("children") };
    const juce::Identifier cmake                   { juce::String::fromUTF8 ("cmake") };
    const juce::Identifier column                  { juce::String::fromUTF8 ("column") };
    const juce::Identifier compilerManifest        { juce::String::fromUTF8 ("compilerManifest") };
    const juce::Identifier compilerManifestCommand { juce::String::fromUTF8 ("compilerManifestCommand") };
    const juce::Identifier compilerManifests       { juce::String::fromUTF8 ("compilerManifests") };
    const juce::Identifier completion              { juce::String::fromUTF8 ("completion") };
    const juce::Identifier connection              { juce::String::fromUTF8 ("connection") };
    const juce::Identifier connections             { juce::String::fromUTF8 ("connections") };
    const juce::Identifier daemon                  { juce::String::fromUTF8 ("daemon") };
    const juce::Identifier declarations            { juce::String::fromUTF8 ("declarations") };
    const juce::Identifier dependencies            { juce::String::fromUTF8 ("dependencies") };
    const juce::Identifier dependencyCommand       { juce::String::fromUTF8 ("dependencyCommand") };
    const juce::Identifier dependencyFlag          { juce::String::fromUTF8 ("dependencyFlag") };
    const juce::Identifier dependencyHead          { juce::String::fromUTF8 ("dependencyHead") };
    const juce::Identifier dependencyRecord        { juce::String::fromUTF8 ("dependencyRecord") };
    const juce::Identifier dependencyRecords       { juce::String::fromUTF8 ("dependencyRecords") };
    const juce::Identifier diagnostics             { juce::String::fromUTF8 ("diagnostics") };
    const juce::Identifier documentSymbol          { juce::String::fromUTF8 ("documentSymbol") };
    const juce::Identifier documents               { juce::String::fromUTF8 ("documents") };
    const juce::Identifier family                  { juce::String::fromUTF8 ("family") };
    const juce::Identifier files                   { juce::String::fromUTF8 ("files") };
    const juce::Identifier findReferences          { juce::String::fromUTF8 ("findReferences") };
    const juce::Identifier flag                    { juce::String::fromUTF8 ("flag") };
    const juce::Identifier flagBuildDirectory      { juce::String::fromUTF8 ("flagBuildDirectory") };
    const juce::Identifier flagDependencyDirectory { juce::String::fromUTF8 ("flagDependencyDirectory") };
    const juce::Identifier goToDefinition          { juce::String::fromUTF8 ("goToDefinition") };
    const juce::Identifier goToImplementation      { juce::String::fromUTF8 ("goToImplementation") };
    const juce::Identifier hash                    { juce::String::fromUTF8 ("hash") };
    const juce::Identifier help                    { juce::String::fromUTF8 ("help") };
    const juce::Identifier hover                   { juce::String::fromUTF8 ("hover") };
    const juce::Identifier incomingCalls           { juce::String::fromUTF8 ("incomingCalls") };
    const juce::Identifier inlayHint               { juce::String::fromUTF8 ("inlayHint") };
    const juce::Identifier jucePath                { juce::String::fromUTF8 ("jucePath") };
    const juce::Identifier juceVersion             { juce::String::fromUTF8 ("juceVersion") };
    const juce::Identifier language                { juce::String::fromUTF8 ("language") };
    const juce::Identifier ling                    { juce::String::fromUTF8 ("ling") };
    const juce::Identifier manifest                { juce::String::fromUTF8 ("manifest") };
    const juce::Identifier module                  { juce::String::fromUTF8 ("module") };
    const juce::Identifier outgoingCalls           { juce::String::fromUTF8 ("outgoingCalls") };
    const juce::Identifier output                  { juce::String::fromUTF8 ("output") };
    const juce::Identifier patch                   { juce::String::fromUTF8 ("patch") };
    const juce::Identifier prepareCallHierarchy    { juce::String::fromUTF8 ("prepareCallHierarchy") };
    const juce::Identifier references              { juce::String::fromUTF8 ("references") };
    const juce::Identifier request                 { juce::String::fromUTF8 ("request") };
    const juce::Identifier result                  { juce::String::fromUTF8 ("result") };
    const juce::Identifier role                    { juce::String::fromUTF8 ("role") };
    const juce::Identifier roots                   { juce::String::fromUTF8 ("roots") };
    const juce::Identifier scope                   { juce::String::fromUTF8 ("scope") };
    const juce::Identifier snippet                 { juce::String::fromUTF8 ("snippet") };
    const juce::Identifier submodule               { juce::String::fromUTF8 ("submodule") };
    const juce::Identifier swap                    { juce::String::fromUTF8 ("swap") };
    const juce::Identifier switchSourceHeader      { juce::String::fromUTF8 ("switchSourceHeader") };
    const juce::Identifier symbol                  { juce::String::fromUTF8 ("symbol") };
    const juce::Identifier timeout                 { juce::String::fromUTF8 ("timeout") };
    const juce::Identifier toolchain               { juce::String::fromUTF8 ("toolchain") };
    const juce::Identifier toolchainArgument       { juce::String::fromUTF8 ("toolchainArgument") };
    const juce::Identifier userModule              { juce::String::fromUTF8 ("@user-module") };
    const juce::Identifier workspaceSymbol         { juce::String::fromUTF8 ("workspaceSymbol") };

/**______________________________END OF NAMESPACE______________________________*/
}// namespace Id
namespace files
{
/*_____________________________________________________________________________*/

/** @brief File, folder, and variable names that ling reads or writes, such as the help and the language files. */

const juce::String castJuceCache           { juce::String::fromUTF8 (".cache/cast/juce") };
const juce::String castJuceLocalCache      { juce::String::fromUTF8 ("cast/juce") };
const juce::String core                    { juce::String::fromUTF8 ("core.md") };
const juce::String defaultLanguage         { juce::String::fromUTF8 ("cLang") };
const juce::String language                { juce::String::fromUTF8 ("language") };
const juce::String languageConstruction    { juce::String::fromUTF8 ("construction.md") };
const juce::String languageInterface       { juce::String::fromUTF8 ("jam_LanguageInterface.h") };
const juce::String languageLibrary         { juce::String::fromUTF8 ("build/liblanguage.dylib") };
const juce::String languageLibraryWindows  { juce::String::fromUTF8 ("build/language.dll") };
const juce::String languageSpell           { juce::String::fromUTF8 ("spell.md") };
const juce::String languageTemplate        { juce::String::fromUTF8 ("language.cast") };
const juce::String lingCache               { juce::String::fromUTF8 (".cache/ling") };
const juce::String lingConfig              { juce::String::fromUTF8 (".config/ling") };
const juce::String lingHelp                { juce::String::fromUTF8 ("HELP.md") };
const juce::String lingLocalCache          { juce::String::fromUTF8 ("ling") };
const juce::String lingPort                { juce::String::fromUTF8 ("port") };
const juce::String lingSwap                { juce::String::fromUTF8 ("swap") };
const juce::String sourceDirectoryVariable { juce::String::fromUTF8 ("${CMAKE_CURRENT_SOURCE_DIR}") };
const juce::String syntaxTree              { juce::String::fromUTF8 ("syntax_tree.md") };
const juce::String userModulesInfo         { juce::String::fromUTF8 ("user-modules-info.md") };

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
