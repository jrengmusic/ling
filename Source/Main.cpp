#include <JuceHeader.h>
#include "generated/Generated.h"
#include "Help.h"
#include "LINGModel.h"
#include "Processor.h"
#include "Server.h"
#include "Client.h"
#include "Manifest.h"
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
 * @brief Returns the table of flags that run without a document: the version and the help.
 *
 * The table is built on the first call. Each entry runs its action and returns
 * the exit code of the run.
 *
 * @return A reference to the static table.
 */
static const jam::Function::Map<juce::Identifier, jam::Function::pointerCapacity, int ()>& getFlags()
{
    static const auto flags (
        []
        {
            jam::Function::Map<juce::Identifier, jam::Function::pointerCapacity, int ()> flags;

            flags.add (Id::version, [] (const juce::Identifier&) { writeVersion(); return EXIT_SUCCESS; });
            flags.add (Id::help, [] (const juce::Identifier&) { printBannerAndHelp(); return EXIT_SUCCESS; });

            return flags;
        }()
    );

    return flags;
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
            if (Processor::isDeclaration (element))
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
 * @brief Resolves a manifest argument against the current working folder, then follows a link to its target.
 *
 * @param arguments      The command line arguments.
 * @param argumentIndex  The index of the manifest argument. It is flagArgIndex for a manifest run, and postFlagArgIndex for an operation run.
 * @return The resolved manifest file.
 */
static juce::File getDocumentFile (const juce::StringArray& arguments, int argumentIndex)
{
    return juce::File::getCurrentWorkingDirectory().getChildFile (arguments[argumentIndex]).getLinkedTarget();
}

/**
 * @brief Returns the word of a flag argument, without its double dash.
 *
 * The double dash test is kept in this one function, so each caller follows the
 * same rule. An argument without a double dash has no flag word.
 *
 * @param argument  The command line argument.
 * @return The text after the double dash, or an empty view when the argument has no double dash.
 */
static std::string_view getFlagWord (std::string_view argument)
{
    const std::string_view doubleDash { Chars::doubleDash };

    if (argument.compare (0, doubleDash.size(), doubleDash) == 0)
        return argument.substr (doubleDash.size());

    return {};
}

/**
 * @brief Returns the toolchain argument that follows the manifest argument, without its double dash.
 *
 * The double dash test is made by getFlagWord. An argument that does not start
 * with a double dash is not a toolchain argument.
 *
 * @param arguments      The command line arguments.
 * @param manifestIndex  The index of the manifest argument.
 * @return The toolchain argument text, or an empty view when the next argument is not one.
 */
static std::string_view getToolchainArgument (const juce::StringArray& arguments, int manifestIndex)
{
    const auto argumentIndex { manifestIndex + 1 };

    if (argumentIndex < arguments.size())
        return getFlagWord (jam::Strings::getView (arguments[argumentIndex]));

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
 * @brief Runs the version or the help flag, reports a manifest as not found, or reports an unknown flag.
 *
 * The flags table selects --version and --help. A manifest argument, with or
 * without a toolchain argument, goes to the not-found report. Any other command
 * line with more than one argument prints the unknown flag error for the
 * argument after the manifest, or for the argument after its toolchain argument.
 * With no argument, the function prints the banner and the help text.
 *
 * @param arguments  The command line arguments.
 * @return @c EXIT_SUCCESS for the version, the help, or no argument, or @c EXIT_FAILURE for a not-found file or an unknown flag.
 */
static int runCommandLine (const juce::StringArray& arguments)
{
    const auto word { arguments.size() == flagOnlyArgumentCount ? getFlagWord (jam::Strings::getView (arguments[flagArgIndex])) : std::string_view {} };
    const juce::Identifier flag { word.empty() ? juce::Identifier {} : juce::Identifier { jam::Strings::fromView (word) } };

    if (getFlags().contains (flag)) return getFlags().get (flag);

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
 * initialise() starts one of three runs: the manifest run, the daemon run, or the operation run.
 * Any other command line runs the command line, which sets the exit code and quits.
 *
 * The manifest run writes the toolchain argument and the manifest path to the model. The processor
 * and the manifest reading then parse the files, and the application prints the documents and quits.
 * The daemon run starts the processor, the manifest reading, and the socket server, when it takes
 * the lock of the daemon. The operation run starts a client that sends one request to the daemon.
 */
class Application : public juce::JUCEApplicationBase,
                    private juce::ValueTree::Listener
{
public:
    /** @brief Creates the message attachment, which prints each overlay message as an error line. */
    Application()
        : message (*model.getParameter<jam::ParameterText> (jam::Format::toType (Id::overlay), Id::message),
                   [] (const juce::var& text) { printError (jam::Strings::getView (getCommandLineParameterArray()[flagArgIndex]), jam::Strings::getView (text.toString())); })
    {
    }

    /** @brief Removes the application from the listeners of the model. */
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
     * @brief Starts the manifest run, the daemon run, or the operation run. Any other argument list runs the command line.
     *
     * The manifest run starts when the first argument names an existing manifest, with at most one
     * toolchain argument after it. The daemon run starts when the first argument is the daemon word
     * and the second argument names an existing file. The operation run starts when the second argument
     * names an existing file and the first argument is not the daemon word. Any other argument list runs
     * the command line, which sets the exit code and quits. On Windows, the console output code page is
     * set to UTF-8. On a terminal, the screen is erased first.
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
        const auto documentFile { getDocumentFile (arguments, flagArgIndex) };

        if (isDocument (arguments) and documentFile.existsAsFile())
        {
            model.addListener (this);
            processor = std::make_unique<Processor> (model);
            manifest = std::make_unique<Manifest> (model);
            model.state.setProperty (Id::toolchainArgument, jam::Strings::fromView (getToolchainArgument (arguments, flagArgIndex)), nullptr);
            model.state.setProperty (Id::manifest, documentFile.getFullPathName(), nullptr);
        }
        else if (getDocumentFile (arguments, postFlagArgIndex).existsAsFile())
        {
            if (arguments[flagArgIndex].compare (Id::daemon.toString()) == 0)
                runDaemon (arguments);
            else
                runOperation (arguments);
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
     * @brief Starts the daemon of the manifest and the toolchain argument, when no other daemon holds its lock.
     *
     * The lock name is the daemon name of the manifest and the toolchain argument. The lock is tried
     * without a wait. When the lock is taken, the function starts the processor and the manifest reading,
     * sets the toolchain argument and the manifest path in the model, and starts the socket server. When
     * the lock is not taken, the application quits.
     *
     * @param arguments  The command line arguments. Index postFlagArgIndex names the manifest.
     */
    void runDaemon (const juce::StringArray& arguments)
    {
        const auto manifestFile { getDocumentFile (arguments, postFlagArgIndex) };
        const auto toolchainArgument { getToolchainArgument (arguments, postFlagArgIndex) };

        lock = std::make_unique<juce::InterProcessLock> (juce::String (LINGModel::getDaemonName (manifestFile, toolchainArgument)));

        if (lock->enter (0))
        {
            processor = std::make_unique<Processor> (model);
            manifest = std::make_unique<Manifest> (model);
            server = std::make_unique<Server>();
            model.state.setProperty (Id::toolchainArgument, jam::Strings::fromView (toolchainArgument), nullptr);
            model.state.setProperty (Id::manifest, manifestFile.getFullPathName(), nullptr);
            server->start();
        }
        else
        {
            quit();
        }
    }

    /**
     * @brief Starts a client that sends one request to the daemon of the manifest.
     *
     * The request is a JSON array. Its first word is the flag argument as typed, its second word is the
     * working folder, and its other words are the arguments after the manifest and the toolchain argument.
     * The client starts a daemon when no daemon answers, and it prints the reply.
     *
     * @param arguments  The command line arguments. Index postFlagArgIndex names the manifest.
     */
    void runOperation (const juce::StringArray& arguments)
    {
        const auto toolchainArgument { getToolchainArgument (arguments, postFlagArgIndex) };
        const int index { postFlagArgIndex + (toolchainArgument.empty() ? flagOnlyArgumentCount : toolchainArgumentCount) };
        std::string request;

        jam::JsonWriter::appendArray (request, [&arguments, index] (std::string& words)
        {
            jam::JsonWriter::appendSeparator (words);
            jam::JsonWriter::appendString (words, jam::Strings::getView (arguments[flagArgIndex]));
            jam::JsonWriter::appendSeparator (words);
            jam::JsonWriter::appendString (words, jam::Strings::getView (juce::File::getCurrentWorkingDirectory().getFullPathName()));

            for (int argumentIndex { index }; argumentIndex < arguments.size(); ++argumentIndex)
            {
                jam::JsonWriter::appendSeparator (words);
                jam::JsonWriter::appendString (words, jam::Strings::getView (arguments[argumentIndex]));
            }
        });

        client = std::make_unique<Client> (getDocumentFile (arguments, postFlagArgIndex), jam::Strings::fromView (toolchainArgument), jam::Strings::fromView (request));
        client->start();
    }

    /**
     * @brief Prints each stored document in path order, then the documents count, and quits.
     *
     * Each document prints as its path, its declaration count, and its reference count. The line of the
     * documents count follows the documents. The function acts only on Id::documents.
     *
     * @param property  The changed property. Only Id::documents acts.
     */
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier& property) override
    {
        if (property == Id::documents)
        {
            std::string rows;

            processor->applyToDocuments ([&rows] (std::string_view path, const jam::CodeDocument& document) { appendDocument (rows, path, document); });
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
    std::unique_ptr<Processor> processor;
    /** @brief The manifest reading. It reads the manifest when the manifest path changes. The manifest run and the daemon run create it. */
    std::unique_ptr<Manifest> manifest;
    /** @brief The attachment that prints each change of the overlay message to stderr. */
    jam::Model::ParameterAttachment message;
    /** @brief The inter-process lock of the daemon name. The daemon run takes it, and it stays held while the daemon runs. */
    std::unique_ptr<juce::InterProcessLock> lock;
    /** @brief The socket server of the daemon. The daemon run creates it. */
    std::unique_ptr<Server> server;
    /** @brief The client of the operation run. It sends the request to the daemon. */
    std::unique_ptr<Client> client;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Application)
};

START_JUCE_APPLICATION (Application)
