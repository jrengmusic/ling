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
    static constexpr const char* const failNotFound                 { "not found"                       };///< A file argument that does not exist.
    static constexpr const char* const failFlagUnknown              { "unknown flag"                    };///< Not --help or --version, or a rejected argument after the manifest.
    static constexpr const char* const failUnitNotFound             { "unit file not found"             };///< A unit file of a compiler manifest that does not exist.
    static constexpr const char* const failCompilerManifestInvalid  { "compiler manifest is not valid"  };///< A compiler manifest whose root is not an array.
    static constexpr const char* const failCompilerManifestNotFound { "compiler manifest not found"     };///< A compiler manifest that does not exist in its build folder.
    static constexpr const char* const failDependenciesNotFound     { "dependencies not found"          };///< A unit with no record in the dependency record.
    static constexpr const char* const failToolchain                { "toolchain command failed"        };///< A toolchain row's process could not start or exited nonzero.
    static constexpr const char* const failToolchainArgument        { "toolchain argument not declared" };///< A --\<word\> CLI argument matched no toolchain row's argument column.
    static constexpr const char* const failToolchainColumn          { "toolchain column not declared"   };///< A ## toolchain table's header row declares no command or flag column.
};

/**______________________________END OF NAMESPACE______________________________*/
}// namespace text
namespace text
{
/*_____________________________________________________________________________*/

/** @brief The words of the C family compiler manifest and dependency record. */
struct CLanguage
{
    static constexpr const char* const compilerManifestCommand { "cmake"                 };///< The command cell of the toolchain row that names the build folder.
    static constexpr const char* const flagBuildDirectory      { "-B"                    };///< The flag whose next token is the build folder.
    static constexpr const char* const compilerManifest        { "compile_commands.json" };///< The file name of the compiler manifest in the build folder.
    static constexpr const char* const dependencyCommand       { "ninja"                 };///< The command cell of the toolchain row that gives the dependency record.
    static constexpr const char* const flagDependencyDirectory { "-C"                    };///< The flag whose next token is the build folder of that row.
    static constexpr const char* const dependencyFlag          { "-t deps"               };///< The flag that makes the command print the dependency record.
    static constexpr const char* const dependencyHead          { ": #deps"               };///< The text that follows the output path in the head line of a record.
};

/**______________________________END OF NAMESPACE______________________________*/
}// namespace text
