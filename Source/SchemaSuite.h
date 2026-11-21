#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

#if LING_USING_SCHEMA
struct SchemaSuite
{
    static juce::Result run (const juce::File& folder);

private:
    static int runCase (const juce::String& fileName, const std::string& fileText, int caseIndex);
    static int runTests (const jam::JsonSchema& schemas, const jam::JsonDocument& document, const juce::String& fileName, const jam::Document::Element& caseParent, const jam::Document::Token& caseToken);
    static void printFailure (const juce::String& fileName, const jam::JsonDocument& document, const jam::Document::Token& caseDescription, const jam::Document::Token& testDescription);
    static juce::String getPointer (const juce::String& parentPointer, std::string_view member);
    static bool isApplicable (const jam::JsonDocument& suite, const jam::Document::Element& testCase);
    static bool isCompatible (std::string_view constraints);

    static constexpr int versionUnderTest { 2020 };
    static constexpr std::string_view caseUri { "urn:jam:suite:case" };
    static constexpr std::string_view atMost { "<=" };
    static constexpr std::string_view exactly { "=" };
    static constexpr std::string_view formatAssertion { "format-assertion" };
    static constexpr std::string_view compatibility { "compatibility" };
    static constexpr std::string_view tags { "tags" };
    static constexpr std::string_view externalSchemas { "externalSchemas" };
    static constexpr std::string_view tests { "tests" };
    static constexpr std::string_view schema { "schema" };
    static constexpr std::string_view data { "data" };
    static constexpr std::string_view valid { "valid" };
    static constexpr std::string_view description { "description" };
    static constexpr std::string_view lineSeparator { ": " };
    static constexpr std::string_view failed { " failed" };
};
#endif
