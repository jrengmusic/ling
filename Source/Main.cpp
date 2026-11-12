#include <JuceHeader.h>
#include "generated/Generated.h"
#include "Help.h"
#include "LINGModel.h"
#include "Processor.h"
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

/**
 * @brief Prints the banner, one line at a time, to stdout.
 */
static void printBanner()
{
    for (const auto& [name, text] : map::banner)
        printf ("%s\n", text.toRawUTF8());
}

/**
 * @brief Writes one error line to stderr: the project name, the argument, and the message.
 *
 * @param argument  The argument that the error concerns.
 * @param message   The error text.
 */
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

static constexpr int flagArgIndex { 0 };
static constexpr int postFlagArgIndex { 1 };
static constexpr int flagOnlyArgumentCount { 1 };
static constexpr int toolchainArgumentCount { 2 };

/**
 * @brief Returns the version flag: the double dash and the version word.
 *
 * @return The version flag, built once on first use.
 */
static const std::string& getVersionFlag()
{
    static const std::string versionFlag (std::string (Chars::doubleDash).append (jam::Strings::getView (Id::version.toString())));
    return versionFlag;
}

/**
 * @brief Returns the help flag: the double dash and the help word.
 *
 * @return The help flag, built once on first use.
 */
static const std::string& getHelpFlag()
{
    static const std::string helpFlag (std::string (Chars::doubleDash).append (jam::Strings::getView (Id::help.toString())));
    return helpFlag;
}

/**
 * @brief Tells whether the arguments are the version request.
 *
 * @param arguments  The command line arguments.
 * @return @c true when the only argument is the version flag.
 */
static bool isVersion (const juce::StringArray& arguments)
{
    return arguments.size() == flagOnlyArgumentCount and jam::Strings::getView (arguments[flagArgIndex]).compare (getVersionFlag()) == 0;
}

/**
 * @brief Tells whether the arguments are the help request.
 *
 * @param arguments  The command line arguments.
 * @return @c true when the only argument is the help flag.
 */
static bool isHelp (const juce::StringArray& arguments)
{
    return arguments.size() == flagOnlyArgumentCount and jam::Strings::getView (arguments[flagArgIndex]).compare (getHelpFlag()) == 0;
}

/**
 * @brief Writes the project name, the version, and the build commit to stdout.
 */
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

/**
 * @brief Tells whether stdout is a terminal.
 *
 * @return @c true when stdout is a terminal.
 */
static bool isTerminalOutput() noexcept
{
#ifdef _WIN32
    return _isatty (_fileno (stdout)) != 0;
#else
    return isatty (fileno (stdout)) != 0;
#endif
}

/**
 * @brief Prints the banner and the help text to stdout.
 *
 * The help text is the binary resource files::lingHelp.
 */
static void printBannerAndHelp()
{
    const BinaryData::Raw binary { files::lingHelp };
    std::string newlineText;
    jam::Format::appendUtf8 (newlineText, Chars::newline);

    printBanner();
    fwrite (newlineText.data(), sizeof (char), newlineText.size(), stdout);
    printHelp (std::string_view (binary.data, static_cast<size_t> (binary.size)));
}

/**
 * @brief Appends one report row for a document: its path, its declaration count, and its reference count.
 *
 * An element whose type is below the reference type is a declaration. Every
 * other element is a reference.
 *
 * @param rows      The text that receives the row.
 * @param path      The path of the document.
 * @param document  The document to count.
 */
static void appendDocument (std::string& rows, std::string_view path, const jam::CodeDocument& document)
{
    int declarations { 0 };
    int references { 0 };

    for (const auto* child : document)
        child->applyFunctionRecursively ([&declarations, &references] (const jam::Document::Element& element)
        {
            if (*element.get<int> (Id::type) < map::SyntaxTokenType::reference)
                ++declarations;
            else
                ++references;
        });

    rows.append (path);
    jam::Format::appendUtf8 (rows, Chars::colon);
    jam::Format::appendUtf8 (rows, Chars::space);
    jam::Format::appendDecimal (rows, declarations);
    jam::Format::appendUtf8 (rows, Chars::space);
    rows.append (jam::Strings::getView (Id::declarations.toString()));
    jam::Format::appendUtf8 (rows, Chars::comma);
    jam::Format::appendUtf8 (rows, Chars::space);
    jam::Format::appendDecimal (rows, references);
    jam::Format::appendUtf8 (rows, Chars::space);
    rows.append (jam::Strings::getView (Id::references.toString()));
    jam::Format::appendUtf8 (rows, Chars::newline);
}

/**
 * @brief Prints the banner and the help text, then reports the path as not found on stderr.
 *
 * @param path  The path that was not found.
 * @return @c EXIT_FAILURE.
 */
static int runDocumentNotFound (std::string_view path)
{
    printBannerAndHelp();
    printError (path, text::Diagnostics::failNotFound);
    return EXIT_FAILURE;
}

/**
 * @brief Resolves the manifest argument against the current working folder, then follows a link to its target.
 *
 * @param arguments  The command line arguments. The manifest argument is at flagArgIndex.
 * @return The resolved manifest file.
 */
static juce::File getDocumentFile (const juce::StringArray& arguments)
{
    return juce::File::getCurrentWorkingDirectory().getChildFile (arguments[flagArgIndex]).getLinkedTarget();
}

/**
 * @brief Returns the toolchain argument that follows the manifest argument, without its double dash.
 *
 * An argument that does not start with a double dash is not a toolchain argument.
 *
 * @param arguments      The command line arguments.
 * @param manifestIndex  The index of the manifest argument.
 * @return The toolchain argument text, or an empty view when the next argument is not one.
 */
static std::string_view getToolchainArgument (const juce::StringArray& arguments, int manifestIndex)
{
    const std::string_view doubleDash { Chars::doubleDash };
    const auto argumentIndex { manifestIndex + 1 };

    if (argumentIndex < arguments.size())
    {
        const std::string_view argument { jam::Strings::getView (arguments[argumentIndex]) };

        if (argument.compare (0, doubleDash.size(), doubleDash) == 0)
            return argument.substr (doubleDash.size());
    }

    return {};
}

/**
 * @brief Tells whether the arguments name a manifest, with or without a toolchain argument.
 *
 * @param arguments  The command line arguments.
 * @return @c true for one argument, or for two arguments whose second argument is a toolchain argument.
 */
static bool isDocument (const juce::StringArray& arguments)
{
    return arguments.size() == flagOnlyArgumentCount
        or (arguments.size() == toolchainArgumentCount and not getToolchainArgument (arguments, flagArgIndex).empty());
}

/**
 * @brief Dispatches the command line to the version, the help, the not-found report, or the unknown flag error.
 *
 * A manifest argument, with or without a toolchain argument, goes to the
 * not-found report. A command line with more than one argument, other than a
 * manifest with a toolchain argument,
 * prints the unknown flag error for the argument after the manifest, or for the
 * argument after its toolchain argument. With no argument, the function prints
 * the banner and the help text.
 *
 * @param arguments  The command line arguments.
 * @return @c EXIT_SUCCESS for the version, the help, or no argument, or @c EXIT_FAILURE for a not-found file or an unknown flag.
 */
static int runCommandLine (const juce::StringArray& arguments)
{
    if (isVersion (arguments))
    {
        writeVersion();
        return EXIT_SUCCESS;
    }

    if (isHelp (arguments))
    {
        printBannerAndHelp();
        return EXIT_SUCCESS;
    }

    if (isDocument (arguments))
        return runDocumentNotFound (jam::Strings::getView (arguments[flagArgIndex]));

    if (arguments.size() > flagOnlyArgumentCount)
    {
        printError (jam::Strings::getView (arguments[getToolchainArgument (arguments, flagArgIndex).empty() ? postFlagArgIndex : toolchainArgumentCount]), text::Diagnostics::failFlagUnknown);
        return EXIT_FAILURE;
    }

    printBannerAndHelp();
    return EXIT_SUCCESS;
}

/**
 * @class Application
 * @brief The juce::JUCEApplicationBase of the LING command line, started by START_JUCE_APPLICATION.
 *
 * initialise() either writes the toolchain argument and the manifest path to
 * the model, or it sets the exit code and quits. The processor writes the
 * documents count, and the application prints each document and quits.
 */
class Application : public juce::JUCEApplicationBase,
                    private juce::ValueTree::Listener
{
public:
    /**
     * @brief Creates the message attachment, which prints each overlay message as an error line, and listens to the model.
     */
    Application()
        : message (*model.getParameter<jam::ParameterText> (jam::Format::toType (Id::overlay), Id::message),
                   [] (const juce::var& text) { printError (jam::Strings::getView (getCommandLineParameterArray()[flagArgIndex]), jam::Strings::getView (text.toString())); })
    {
        model.addListener (this);
    }

    /** @brief Stops listening to the model. */
    ~Application() override
    {
        model.removeListener (this);
    }

    /**
     * @brief Returns the project name.
     *
     * @return The project name.
     */
    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    /**
     * @brief Returns the project version string.
     *
     * @return The project version string.
     */
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    /**
     * @brief Allows more than one instance to run.
     *
     * @return @c true.
     */
    bool moreThanOneInstanceAllowed() override { return true; }

    /**
     * @brief Starts the run. It sets the toolchain argument and the manifest path when the arguments name an existing manifest, or it runs the command line and quits.
     *
     * On Windows, the console output code page is set to UTF-8. On a terminal,
     * the screen is erased before the run.
     */
    void initialise (const juce::String&) override
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

        const auto arguments (getCommandLineParameterArray());
        const auto documentFile { getDocumentFile (arguments) };

        if (isDocument (arguments) and documentFile.existsAsFile())
        {
            model.state.setProperty (Id::toolchainArgument, jam::Strings::fromView (getToolchainArgument (arguments, flagArgIndex)), nullptr);
            model.state.setProperty (Id::manifest, documentFile.getFullPathName(), nullptr);
        }
        else
        {
            setApplicationReturnValue (runCommandLine (arguments));
            quit();
        }
    }

    /** @brief Does nothing. The members release their own resources. */
    void shutdown() override {}
    /** @brief Does nothing. */
    void anotherInstanceStarted (const juce::String&) override {}
    /** @brief Quits the application when the system requests a quit. */
    void systemRequestedQuit() override { quit(); }
    /** @brief Does nothing. */
    void suspended() override {}
    /** @brief Does nothing. */
    void resumed() override {}

    /** @brief Asserts on an exception that reaches the application. */
    void unhandledException (const std::exception*, const juce::String&, int) override
    {
        jassertfalse;
    }

private:
    /**
     * @brief Prints each document, then the documents count, and quits when the documents count is written.
     *
     * The processor writes Id::documents when the last parse job ends. The rows
     * are printed in path order, one row for each document.
     *
     * @param property  The changed property. Only Id::documents prints and quits.
     */
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier& property) override
    {
        if (property == Id::documents)
        {
            std::string rows;

            processor.applyToDocuments ([&rows] (std::string_view path, const jam::CodeDocument& document) { appendDocument (rows, path, document); });
            jam::Format::appendDecimal (rows, static_cast<int> (model.state[Id::documents]));
            jam::Format::appendUtf8 (rows, Chars::space);
            rows.append (jam::Strings::getView (Id::documents.toString()));
            jam::Format::appendUtf8 (rows, Chars::newline);
            fwrite (rows.data(), sizeof (char), rows.size(), stdout);
            quit();
        }
    }

    /** @brief The state model of the run. The application owns it. */
    LINGModel model;
    /** @brief The processor that parses the files of the model. It holds the model by reference. */
    Processor processor { model };
    /** @brief The attachment that prints each change of the overlay message to stderr. */
    jam::Model::ParameterAttachment message;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Application)
};

START_JUCE_APPLICATION (Application)
