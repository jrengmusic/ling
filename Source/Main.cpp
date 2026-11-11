#include <JuceHeader.h>
#include "generated/Generated.h"
#include "Help.h"
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

static void printBanner()
{
    for (const auto& [name, text] : map::banner)
        printf ("%s\n", text.toRawUTF8());
}

static void printError (std::string_view argument, std::string_view message)
{
    std::string errorLine { ProjectInfo::projectName };
    errorLine.append (jam::Strings::getView (Id::diagnosticSeparator));
    errorLine.append (argument);
    errorLine.append (jam::Strings::getView (Id::diagnosticSeparator));
    errorLine.append (message);
    jam::Format::appendUtf8 (errorLine, Chars::newline);
    fwrite (errorLine.data(), sizeof (char), errorLine.size(), stderr);
}

static constexpr int flagArgIndex { 1 };
static constexpr int postFlagArgIndex { 2 };
static constexpr int flagOnlyArgumentCount { 2 };

static const std::string& getVersionFlag()
{
    static const std::string versionFlag (std::string (Chars::doubleDash).append (jam::Strings::getView (Id::version.toString())));
    return versionFlag;
}

static const std::string& getHelpFlag()
{
    static const std::string helpFlag (std::string (Chars::doubleDash).append (jam::Strings::getView (Id::help.toString())));
    return helpFlag;
}

static bool isVersion (int argc, char* argv[])
{
    return argc == flagOnlyArgumentCount and std::string_view (argv[flagArgIndex]).compare (getVersionFlag()) == 0;
}

static bool isHelp (int argc, char* argv[])
{
    return argc == flagOnlyArgumentCount and std::string_view (argv[flagArgIndex]).compare (getHelpFlag()) == 0;
}

static void writeVersion()
{
    std::string versionLine { ProjectInfo::projectName };
    jam::Format::appendUtf8 (versionLine, Chars::space);
    versionLine.append (ProjectInfo::versionString);
    jam::Format::appendUtf8 (versionLine, Chars::space);
    jam::Format::appendUtf8 (versionLine, Chars::openParen);
    versionLine.append (CAST_COMMIT);
    jam::Format::appendUtf8 (versionLine, Chars::closeParen);
    jam::Format::appendUtf8 (versionLine, Chars::newline);
    fwrite (versionLine.data(), sizeof (char), versionLine.size(), stdout);
}

static bool isTerminalOutput() noexcept
{
#ifdef _WIN32
    return _isatty (_fileno (stdout)) != 0;
#else
    return isatty (fileno (stdout)) != 0;
#endif
}

static void printBannerAndHelp()
{
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;
    const BinaryData::Raw binary { files::lingHelp };
    std::string newlineText;
    jam::Format::appendUtf8 (newlineText, Chars::newline);

    printBanner();
    fwrite (newlineText.data(), sizeof (char), newlineText.size(), stdout);
    printHelp (std::string_view (binary.data, static_cast<size_t> (binary.size)));
}

static int runDocument (const juce::File& documentFile, std::string_view path)
{
    const auto sourceText { documentFile.loadFileAsString() };
    const auto document { jam::CLanguageDocument::parse (jam::Strings::getView (sourceText)) };
    const auto result { jam::CLanguageValidator().isValid (document) };

    if (result.wasOk())
    {
        const jam::SharedInstance<map::SyntaxTokenType> syntaxTokenType { std::in_place };
        std::string declarations;

        for (const auto* child : document)
            child->applyFunctionRecursively (
                [&declarations, &document, path] (const jam::Document::Element& element)
                {
                    const auto offset { element.find<jam::Document::Tokens> (Id::name)->last().offset() };

                    declarations.append (path);
                    jam::Format::appendUtf8 (declarations, Chars::colon);
                    jam::Format::appendDecimal (declarations, static_cast<int> (document.getLineNumber (offset)));
                    jam::Format::appendUtf8 (declarations, Chars::colon);
                    jam::Format::appendDecimal (declarations, static_cast<int> (document.getColumnNumber (offset)));
                    jam::Format::appendUtf8 (declarations, Chars::colon);
                    jam::Format::appendUtf8 (declarations, Chars::space);
                    declarations.append (jam::Strings::getView (map::SyntaxTokenType::getInstance()->get (*element.find<int> (Id::type))));
                    jam::Format::appendUtf8 (declarations, Chars::space);
                    declarations.append (jam::Strings::getView (element.id.toString()));
                    jam::Format::appendUtf8 (declarations, Chars::newline);
                });

        fwrite (declarations.data(), sizeof (char), declarations.size(), stdout);
        return EXIT_SUCCESS;
    }

    printError (path, jam::Strings::getView (result.getErrorMessage()));
    return EXIT_FAILURE;
}

static int runDocumentNotFound (std::string_view path)
{
    printBannerAndHelp();
    printError (path, text::Diagnostics::failNotFound);
    return EXIT_FAILURE;
}

static juce::File getDocumentFile (char* argv[])
{
    return juce::File::getCurrentWorkingDirectory().getChildFile (juce::String::fromUTF8 (argv[flagArgIndex])).getLinkedTarget();
}

static int runCommandLine (int argc, char* argv[])
{
    if (isVersion (argc, argv))
    {
        writeVersion();
        return EXIT_SUCCESS;
    }

    if (isHelp (argc, argv))
    {
        printBannerAndHelp();
        return EXIT_SUCCESS;
    }

    if (argc == flagOnlyArgumentCount)
    {
        const auto documentFile { getDocumentFile (argv) };

        if (documentFile.existsAsFile())
            return runDocument (documentFile, argv[flagArgIndex]);

        return runDocumentNotFound (argv[flagArgIndex]);
    }

    if (argc > flagOnlyArgumentCount)
    {
        printError (argv[postFlagArgIndex], text::Diagnostics::failFlagUnknown);
        return EXIT_FAILURE;
    }

    printBannerAndHelp();
    return EXIT_SUCCESS;
}

int main (int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP (CP_UTF8);
#endif

    if (isTerminalOutput())
    {
        juce::MemoryOutputStream erase;
        jam::TerminalEncoder::writeEraseScreen (erase);
        fwrite (erase.getData(), sizeof (char), erase.getDataSize(), stdout);
        fflush (stdout);
    }

    return runCommandLine (argc, argv);
}
