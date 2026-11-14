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
    static constexpr const char* const failNotFound                 { "not found"                                                       };///< A file argument that does not exist.
    static constexpr const char* const failFlagUnknown              { "unknown flag"                                                    };///< Not --help or --version, or a rejected argument after the manifest.
    static constexpr const char* const failArguments                { "wrong arguments"                                                 };///< An operation word that ling does not hold, or a wrong count or value of arguments.
    static constexpr const char* const failUnitNotFound             { "unit file not found"                                             };///< A unit file of a compiler manifest that does not exist.
    static constexpr const char* const failNoUnit                   { "no unit compiles the file"                                       };///< A file that no unit of the compiler manifest compiles.
    static constexpr const char* const failCompilerManifestInvalid  { "compiler manifest is not valid"                                  };///< A compiler manifest whose root is not an array.
    static constexpr const char* const failCompilerManifestNotFound { "compiler manifest not found"                                     };///< A compiler manifest that does not exist in its build folder.
    static constexpr const char* const failDependenciesNotFound     { "dependencies not found"                                          };///< A unit with no record in the dependency record.
    static constexpr const char* const failToolchain                { "toolchain command failed"                                        };///< A toolchain row's process could not start or exited nonzero.
    static constexpr const char* const failToolchainArgument        { "toolchain argument not declared"                                 };///< A --\<word\> CLI argument matched no toolchain row's argument column.
    static constexpr const char* const failToolchainColumn          { "toolchain column not declared"                                   };///< A ## toolchain table's header row declares no command or flag column.
    static constexpr const char* const failDaemonStart              { "daemon did not start"                                            };///< A daemon that did not start.
    static constexpr const char* const failDaemonLost               { "daemon connection lost"                                          };///< A connection to the daemon that ended before the result.
    static constexpr const char* const failDaemonTimeout            { "daemon timeout must be a positive integer"                       };///< A ## daemon timeout cell is not a positive integer.
    static constexpr const char* const failSwapVersion              { "swap of another version: remove the swap folder and build again" };///< A swap file that another version of LING wrote.
    static constexpr const char* const failLanguageBuild            { "language build failed"                                           };///< A language folder whose cast run exited nonzero.
    static constexpr const char* const failLanguageLoad             { "language library did not load"                                   };///< A language library that does not open or has no entry.
    static constexpr const char* const failUnitIndex                { "unit index failed"                                               };///< A unit whose index call returned nonzero.
    static constexpr const char* const failConstructionNotFound     { "no construction has a compilerManifest table"                    };///< No construction file of the language folder holds a compilerManifest table.
};

/**______________________________END OF NAMESPACE______________________________*/
}// namespace text
