#include "SchemaSuite.h"

#if LING_USING_SCHEMA
juce::Result SchemaSuite::run (const juce::File& folder)
{
    auto files (folder.findChildFiles (juce::File::findFiles, false, "*.json"));
    files.sort();
    int numFailures { 0 };

    for (const juce::File& file : files)
    {
        const std::string fileText { file.loadFileAsString().toStdString() };
        const auto suite { jam::JsonDocument::parse (std::string (fileText)) };
        jassert (suite != nullptr);
        const auto [casesParent, cases] { jam::JsonPointer::get (*suite, *suite->getRoot(), suite->getValue(), jam::Strings::getView (getPointer ({}, tests))) };
        const jam::Document::Element& casesElement { *suite->getChildByToken (*casesParent, *cases) };
        const jam::Document::Tokens& caseTokens { *casesElement.get<jam::Document::Tokens> (Id::tokens) };

        for (int caseIndex { 0 }; caseIndex < caseTokens.size(); ++caseIndex)
        {
            const jam::Document::Element& testCase { *suite->getChildByToken (casesElement, caseTokens.at (caseIndex)) };

            if (isApplicable (*suite, testCase))
                numFailures += runCase (file.getFileName(), fileText, caseIndex);
        }
    }

    return numFailures == 0 ? juce::Result::ok() : juce::Result::fail (juce::String (numFailures) + jam::Strings::fromView (failed));
}

int SchemaSuite::runCase (const juce::String& fileName, const std::string& fileText, int caseIndex)
{
    jam::JsonSchema schemas;
    const int index { schemas.add (jam::JsonDocument::parse (std::string (fileText))) };
    const jam::JsonDocument& document { schemas.getDocument (index) };
    const juce::String casePointer { getPointer (getPointer ({}, tests), jam::Strings::getView (juce::String (caseIndex))) };
    const auto [caseParent, caseToken] { jam::JsonPointer::get (document, *document.getRoot(), document.getValue(), jam::Strings::getView (casePointer)) };
    const jam::Document::Element& testCase { *document.getChildByToken (*caseParent, *caseToken) };

    if (document.contains (testCase, externalSchemas))
    {
        const auto& externalToken { document.get (testCase, externalSchemas) };
        const jam::Document::Tokens& keyTokens { *document.getChildByToken (testCase, externalToken)->get<jam::Document::Tokens> (Id::tokens) };

        for (int keyIndex { 0 }; keyIndex < keyTokens.size(); keyIndex += jam::JsonDocument::numMemberTokens)
        {
            const std::string key { document.getString (keyTokens.at (keyIndex)) };
            const juce::String externalPointer { getPointer (getPointer (casePointer, externalSchemas), jam::JsonPointer::getEscapedText (key)) };
            schemas.addSchema (index, jam::Strings::getView (externalPointer), jam::Strings::fromView (key));
        }
    }

    schemas.addSchema (index, jam::Strings::getView (getPointer (casePointer, schema)), jam::Strings::fromView (caseUri));

    return runTests (schemas, document, fileName, *caseParent, *caseToken);
}

int SchemaSuite::runTests (const jam::JsonSchema& schemas, const jam::JsonDocument& document, const juce::String& fileName, const jam::Document::Element& caseParent, const jam::Document::Token& caseToken)
{
    const auto [testsParent, testsToken] { jam::JsonPointer::get (document, caseParent, caseToken, jam::Strings::getView (getPointer ({}, tests))) };
    const jam::Document::Element& testsElement { *document.getChildByToken (*testsParent, *testsToken) };
    const jam::Document::Tokens& testTokens { *testsElement.get<jam::Document::Tokens> (Id::tokens) };
    const auto [caseDescriptionParent, caseDescriptionToken] { jam::JsonPointer::get (document, caseParent, caseToken, jam::Strings::getView (getPointer ({}, description))) };
    int numFailures { 0 };

    for (int testIndex { 0 }; testIndex < testTokens.size(); ++testIndex)
    {
        const auto [dataParent, dataToken] { jam::JsonPointer::get (document, testsElement, testTokens.at (testIndex), jam::Strings::getView (getPointer ({}, data))) };
        const auto [validParent, validToken] { jam::JsonPointer::get (document, testsElement, testTokens.at (testIndex), jam::Strings::getView (getPointer ({}, valid))) };
        const auto [testDescriptionParent, testDescriptionToken] { jam::JsonPointer::get (document, testsElement, testTokens.at (testIndex), jam::Strings::getView (getPointer ({}, description))) };
        const juce::Result result { schemas.isValid (document, *dataParent, *dataToken, jam::Strings::fromView (caseUri)) };

        if (result.wasOk() != document.getBoolean (*validToken))
        {
            printFailure (fileName, document, *caseDescriptionToken, *testDescriptionToken);
            ++numFailures;
        }
    }

    return numFailures;
}

void SchemaSuite::printFailure (const juce::String& fileName, const jam::JsonDocument& document, const jam::Document::Token& caseDescription, const jam::Document::Token& testDescription)
{
    std::string rows;
    rows.append (jam::Strings::getView (fileName));
    rows.append (lineSeparator);
    rows.append (document.getString (caseDescription));
    rows.append (lineSeparator);
    rows.append (document.getString (testDescription));
    jam::Format::appendUtf8 (rows, Chars::newline);
    fwrite (rows.data(), sizeof (char), rows.size(), stdout);
}

juce::String SchemaSuite::getPointer (const juce::String& parentPointer, std::string_view member)
{
    return parentPointer + juce::String::charToString (Chars::slash) + jam::Strings::fromView (member);
}

bool SchemaSuite::isApplicable (const jam::JsonDocument& suite, const jam::Document::Element& testCase)
{
    if (suite.contains (testCase, tags))
    {
        const auto& tagsToken { suite.get (testCase, tags) };
        const jam::Document::Tokens& tagTokens { *suite.getChildByToken (testCase, tagsToken)->get<jam::Document::Tokens> (Id::tokens) };

        for (int tagIndex { 0 }; tagIndex < tagTokens.size(); ++tagIndex)
        {
            if (suite.getString (tagTokens.at (tagIndex)).compare (formatAssertion) == 0)
                return false;
        }
    }

    if (suite.contains (testCase, compatibility))
    {
        const auto& compatibilityToken { suite.get (testCase, compatibility) };

        return isCompatible (suite.getString (compatibilityToken));
    }

    return true;
}

bool SchemaSuite::isCompatible (std::string_view constraints)
{
    bool isCompatibleVersion { true };

    for (size_t start { 0 }; start <= constraints.size();)
    {
        const size_t end { juce::jmin (constraints.find (static_cast<char> (Chars::comma), start), constraints.size()) };
        const std::string_view constraint { constraints.substr (start, end - start) };
        const bool isAtMost { constraint.compare (0, atMost.size(), atMost) == 0 };
        const bool isExactly { not isAtMost and constraint.compare (0, exactly.size(), exactly) == 0 };
        const int version { jam::Format::getNumber<int> (constraint.substr (isAtMost ? atMost.size() : (isExactly ? exactly.size() : 0))) };
        isCompatibleVersion = isAtMost ? versionUnderTest <= version : (isExactly ? versionUnderTest == version : versionUnderTest >= version);

        if (versionUnderTest <= version)
            return isCompatibleVersion;

        start = end + 1;
    }

    return isCompatibleVersion;
}
#endif
