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
 * @file Text.h
 * @brief ling message strings — one constant per failure.
 */

#pragma once

namespace text
{
/*_____________________________________________________________________________*/

/** @brief ling message texts, emitted verbatim when a run fails. */
struct Diagnostics
{
    static constexpr const char* const failNotFound             { "not found"                                                       };///< A file argument that does not exist.
    static constexpr const char* const failFlagUnknown          { "unknown flag"                                                    };///< Not --help or --version, or a rejected argument after the manifest.
    static constexpr const char* const failToolchainArgument    { "toolchain argument not declared"                                 };///< A --\<word\> CLI argument matched no toolchain row's argument column.
    static constexpr const char* const failToolchainColumn      { "toolchain column not declared"                                   };///< A ## toolchain table's header row declares no command or flag column.
    static constexpr const char* const failDaemonStart          { "daemon did not start"                                            };///< A daemon that did not start.
    static constexpr const char* const failDaemonLost           { "daemon connection lost"                                          };///< A connection to the daemon that ended before the result.
    static constexpr const char* const failDaemonTimeout        { "daemon timeout must be a positive integer"                       };///< A ## daemon timeout cell is not a positive integer.
    static constexpr const char* const failDaemonThreads        { "daemon threads must be a positive integer"                       };///< A ## daemon threads cell is not a positive integer.
    static constexpr const char* const failSwapVersion          { "swap of another version: remove the swap folder and build again" };///< A swap file that another version of LING wrote.
    static constexpr const char* const failSwapRecord           { "swap record incomplete"                                          };///< A swap file whose root or record misses a field.
    static constexpr const char* const failLanguageLoad         { "language library did not load"                                   };///< A language library that does not open or has no entry.
    static constexpr const char* const failLanguageParse        { "file did not parse"                                              };///< A file that its language did not parse.
    static constexpr const char* const failUnitIndex            { "unit index failed"                                               };///< A unit whose index call returned nonzero.
    static constexpr const char* const failConstructionNotFound { "no language folder holds a construction file"                    };///< No folder of the language folder holds construction.md.
    static constexpr const char* const failNoDaemon             { "no daemon for manifest"                                          };///< A manifest whose daemon exited with no indexable files.
};
/** @brief ling code action titles. */
struct CodeAction
{
    static constexpr const char* const fixItTitle { "apply fix-it" };
};
/** @brief ling work done progress texts of the index. */
struct Progress
{
    static constexpr const char* const progressToken     { "ling/index" };
    static constexpr const char* const progressTitle     { "Indexing"   };
    static constexpr const char* const progressSeparator { "/"          };
};

/**______________________________END OF NAMESPACE______________________________*/
}// namespace text
