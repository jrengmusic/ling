#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

#if LING_USING_SCHEMA
/**
 * @struct SchemaSuite
 * @brief Executes JSON Schema 2020-12 compliance test suite files from disk when LING_USING_SCHEMA is enabled.
 */
struct SchemaSuite
{
    /**
     * @brief Runs every applicable test case across sorted `.json` files in @p folder.
     *
     * @param folder  Directory containing JSON Schema test suite files.
     * @return juce::Result::ok() when zero tests fail, or a failure with the failure count.
     */
    static juce::Result run (const juce::File& folder);

private:
    /**
     * @brief Loads external schemas and the case schema for one test case, then evaluates its tests.
     *
     * @param fileName   Suite file name used in failure output.
     * @param fileText   Raw JSON text of the suite file.
     * @param caseIndex  Zero-based index of the test case inside the top-level tests array.
     * @return Number of failing test assertions in this case.
     */
    static int runCase (const juce::String& fileName, const std::string& fileText, int caseIndex);

    /**
     * @brief Validates each data instance in a test case against the compiled case schema.
     *
     * @param schemas     Schema registry holding the case schema and any external schemas.
     * @param document    Parsed JSON document of the suite file.
     * @param fileName    Suite file name used in failure output.
     * @param caseParent  Parent element owning @p caseToken.
     * @param caseToken   Token pointing to the test case object.
     * @return Number of failing test assertions in this case.
     */
    static int runTests (const jam::JsonSchema& schemas, const jam::JsonDocument& document, const juce::String& fileName, const jam::Document::Element& caseParent, const jam::Document::Token& caseToken);

    /**
     * @brief Prints one formatted failure line to stdout for a mismatched test assertion.
     *
     * @param fileName         Suite file name.
     * @param document         Parsed JSON document holding the description tokens.
     * @param caseDescription  Token of the enclosing test case description.
     * @param testDescription  Token of the failing test description.
     */
    static void printFailure (const juce::String& fileName, const jam::JsonDocument& document, const jam::Document::Token& caseDescription, const jam::Document::Token& testDescription);

    /**
     * @brief Appends a `/` segment for @p member onto @p parentPointer.
     *
     * @param parentPointer  Base JSON Pointer string (empty for root).
     * @param member         Escaped member name or array index segment.
     * @return Combined JSON Pointer string.
     */
    static juce::String getPointer (const juce::String& parentPointer, std::string_view member);

    /**
     * @brief Tells whether @p testCase applies to draft 2020-12 without format assertions.
     *
     * @param suite     Parsed JSON document of the test suite.
     * @param testCase  Test case object element to inspect.
     * @return @c true when the case should execute.
     */
    static bool isApplicable (const jam::JsonDocument& suite, const jam::Document::Element& testCase);

    /**
     * @brief Evaluates comma-separated draft version constraints against draft 2020.
     *
     * @param constraints  Constraint list such as `2019,<=2020` or `=2020`.
     * @return @c true when version 2020 satisfies @p constraints.
     */
    static bool isCompatible (std::string_view constraints);
};
#endif
