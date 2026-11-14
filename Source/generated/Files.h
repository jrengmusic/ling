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
 * @file Files.h
 * @brief ling's own file names, referenced by the program.
 */

#pragma once

namespace files
{
/*_____________________________________________________________________________*/

/** @brief File, folder, and variable names that ling reads or writes, such as the help and the language files. */

extern const juce::String castJuceCache;          ///< Cache folder of the patched JUCE of cast below the home folder.
extern const juce::String castJuceLocalCache;     ///< Cache folder of the patched JUCE of cast in %LOCALAPPDATA%.
extern const juce::String core;                   ///< Character and token table file of a language build.
extern const juce::String defaultLanguage;        ///< Name of the language whose files LING embeds.
extern const juce::String language;               ///< Folder of the language folders, below the config folder and below the cache folder.
extern const juce::String languageConstruction;   ///< Construction table file of a language.
extern const juce::String languageInterface;      ///< Boundary header of a language library.
extern const juce::String languageLibrary;        ///< Library that a language build writes, below its cache folder.
extern const juce::String languageLibraryWindows; ///< Library that a language build writes on Windows, below its cache folder.
extern const juce::String languageSpell;          ///< Spell file of a language.
extern const juce::String languageTemplate;       ///< Template file of a language.
extern const juce::String lingCache;              ///< Cache folder of LING below the home folder or a repository root.
extern const juce::String lingConfig;             ///< Config folder of LING below the home folder.
extern const juce::String lingHelp;               ///< Rendered help text.
extern const juce::String lingLocalCache;         ///< Cache folder of LING in %LOCALAPPDATA%.
extern const juce::String lingPort;               ///< Extension of the port file of a daemon.
extern const juce::String lingSwap;               ///< Extension of the swap file of a source file.
extern const juce::String sourceDirectoryVariable;///< The CMake variable that a manifest cell uses for the project folder.
extern const juce::String syntaxTree;             ///< Syntax table file of a language build.
extern const juce::String userModulesInfo;        ///< File name of the module table in the user module folder.

/**______________________________END OF NAMESPACE______________________________*/
}// namespace files
