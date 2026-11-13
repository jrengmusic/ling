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

/** @brief File names ling reads: help. */

extern const juce::String lingCache;     ///< Cache folder of LING below the home folder or a repository root.
extern const juce::String lingHelp;      ///< Rendered help text.
extern const juce::String lingLocalCache;///< Cache folder of LING in %LOCALAPPDATA%.
extern const juce::String lingPort;      ///< Extension of the port file of a daemon.

/**______________________________END OF NAMESPACE______________________________*/
}// namespace files
