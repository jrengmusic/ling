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

    const juce::Identifier argument             { juce::String::fromUTF8 ("argument") };
    const juce::Identifier arguments            { juce::String::fromUTF8 ("arguments") };
    const juce::Identifier children             { juce::String::fromUTF8 ("children") };
    const juce::Identifier column               { juce::String::fromUTF8 ("column") };
    const juce::Identifier compilerManifests    { juce::String::fromUTF8 ("compilerManifests") };
    const juce::Identifier connection           { juce::String::fromUTF8 ("connection") };
    const juce::Identifier connections          { juce::String::fromUTF8 ("connections") };
    const juce::Identifier daemon               { juce::String::fromUTF8 ("daemon") };
    const juce::Identifier declarations         { juce::String::fromUTF8 ("declarations") };
    const juce::Identifier dependencies         { juce::String::fromUTF8 ("dependencies") };
    const juce::Identifier dependencyRecords    { juce::String::fromUTF8 ("dependencyRecords") };
    const juce::Identifier documents            { juce::String::fromUTF8 ("documents") };
    const juce::Identifier documentSymbol       { juce::String::fromUTF8 ("documentSymbol") };
    const juce::Identifier family               { juce::String::fromUTF8 ("family") };
    const juce::Identifier files                { juce::String::fromUTF8 ("files") };
    const juce::Identifier findReferences       { juce::String::fromUTF8 ("findReferences") };
    const juce::Identifier flag                 { juce::String::fromUTF8 ("flag") };
    const juce::Identifier goToDefinition       { juce::String::fromUTF8 ("goToDefinition") };
    const juce::Identifier goToImplementation   { juce::String::fromUTF8 ("goToImplementation") };
    const juce::Identifier help                 { juce::String::fromUTF8 ("help") };
    const juce::Identifier hover                { juce::String::fromUTF8 ("hover") };
    const juce::Identifier incomingCalls        { juce::String::fromUTF8 ("incomingCalls") };
    const juce::Identifier ling                 { juce::String::fromUTF8 ("ling") };
    const juce::Identifier manifest             { juce::String::fromUTF8 ("manifest") };
    const juce::Identifier outgoingCalls        { juce::String::fromUTF8 ("outgoingCalls") };
    const juce::Identifier output               { juce::String::fromUTF8 ("output") };
    const juce::Identifier prepareCallHierarchy { juce::String::fromUTF8 ("prepareCallHierarchy") };
    const juce::Identifier references           { juce::String::fromUTF8 ("references") };
    const juce::Identifier request              { juce::String::fromUTF8 ("request") };
    const juce::Identifier result               { juce::String::fromUTF8 ("result") };
    const juce::Identifier scope                { juce::String::fromUTF8 ("scope") };
    const juce::Identifier timeout              { juce::String::fromUTF8 ("timeout") };
    const juce::Identifier toolchain            { juce::String::fromUTF8 ("toolchain") };
    const juce::Identifier toolchainArgument    { juce::String::fromUTF8 ("toolchainArgument") };
    const juce::Identifier workspaceSymbol      { juce::String::fromUTF8 ("workspaceSymbol") };

/**______________________________END OF NAMESPACE______________________________*/
}// namespace Id
namespace files
{
/*_____________________________________________________________________________*/

/** @brief File names ling reads: help. */

const juce::String lingCache      { juce::String::fromUTF8 (".cache/ling") };
const juce::String lingHelp       { juce::String::fromUTF8 ("HELP.md") };
const juce::String lingLocalCache { juce::String::fromUTF8 ("ling") };
const juce::String lingPort       { juce::String::fromUTF8 ("port") };

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
