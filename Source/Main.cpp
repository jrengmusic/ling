#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Processor.h"
#include "Server.h"
#include "Operation.h"
#include "Relay.h"
#include "ToolRelay.h"
#include "Manifest.h"
#include "SchemaSuite.h"
#include "StdioClientProbe.h"
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
 * @brief Returns one error line: the project name, the argument, and the message, without a trailing newline.
 *
 * @param argument  The argument that the error concerns.
 * @param message   The error text.
 * @return The error line.
 */
static juce::String getErrorLine (std::string_view argument, std::string_view message)
{
    std::string errorLine { ProjectInfo::projectName };
    errorLine.append (jam::Strings::getView (Id::diagnosticSeparator));
    errorLine.append (argument);
    errorLine.append (jam::Strings::getView (Id::diagnosticSeparator));
    errorLine.append (message);
    return jam::Strings::fromView (errorLine);
}

/**
 * @brief Writes one error line to stderr: the project name, the argument, and the message.
 *
 * @param argument  The argument that the error concerns.
 * @param message   The error text.
 */
static void printError (std::string_view argument, std::string_view message)
{
    std::string errorLine { jam::Strings::getView (getErrorLine (argument, message)) };
    jam::Format::appendUtf8 (errorLine, Chars::newline);
    fwrite (errorLine.data(), sizeof (char), errorLine.size(), stderr);
}

static constexpr int commandIndex { 0 };
static constexpr int manifestIndex { 1 };

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
    jam::TerminalPage page { { BinaryData::fetcher }, jam::Strings { files::styleSheet } };
    juce::MemoryOutputStream frame;

    printBanner();
    fwrite (newlineText.data(), sizeof (char), newlineText.size(), stdout);
    page.setDocument (std::move (*jam::MarkdownDocument::parse (std::string_view (binary.data, static_cast<size_t> (binary.size)), files::lingHelp)));
    page.writeToStream (frame);
    fwrite (frame.getData(), sizeof (char), frame.getDataSize(), stdout);
}

#if JAM_USING_PROFILER
/**
 * @brief Appends one report row for a document: its path, its declaration count, and its reference count.
 *
 * An element with the declaration role is a declaration. Every other element is a reference.
 *
 * @param rows      The text that receives the row.
 * @param path      The path of the document.
 * @param document  The document to count.
 */
static void appendDocument (std::string& rows, std::string_view path, const jam::Document& document, const Index& index)
{
    int declarations { 0 };
    int references { 0 };

    for (const auto* child : document)
        child->applyFunctionRecursively ([&document, &index, &declarations, &references] (const jam::Document::Element& element)
        {
            if (index.hasRole (document, element, jam::LanguageRole::declaration))
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
#endif

static juce::File getManifestFile (const juce::ArgumentList& arguments, int argumentIndex)
{
    const auto argumentFile { arguments[argumentIndex].resolveAsFile() };
    const auto workingFolder { juce::File::getCurrentWorkingDirectory() };

    if (argumentFile.existsAsFile())
        return argumentFile.getLinkedTarget();

    if (argumentFile.getFileName().compare (arguments[argumentIndex].text) == 0)
    {
        const auto manifestFile { LINGModel::getManifestFile (workingFolder, argumentFile.getFileName()) };

        return manifestFile != juce::File {} ? manifestFile : workingFolder;
    }

    return argumentFile;
}

static juce::File getExistingManifestFile (const juce::ArgumentList& arguments, int argumentIndex)
{
    const auto manifestFile { getManifestFile (arguments, argumentIndex) };

    if (manifestFile.exists())
        return manifestFile;

    juce::ConsoleApplication::fail (getErrorLine (jam::Strings::getView (arguments[argumentIndex].text), text::Diagnostics::failNotFound), EXIT_FAILURE);
}

static juce::String getToolchainArgument (const juce::ArgumentList& arguments, int argumentIndex)
{
    const auto toolchainIndex { argumentIndex + 1 };

    if (toolchainIndex < arguments.size())
    {
        const auto argument { arguments[toolchainIndex] };

        if (argument.isLongOption())
            return argument.text.trimCharactersAtStart (Chars::doubleDash);
    }

    return {};
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
                   [this] (const juce::var& text)
                   {
                       printError (jam::Strings::getView (getCommandLineParameterArray()[commandIndex]), jam::Strings::getView (text.toString()));
                   })
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

        addCommands();

        const auto exitCode { commands.findAndRunCommand ({ ProjectInfo::projectName, getCommandLineParameterArray() }, true) };

        if (exitCode != EXIT_SUCCESS)
        {
            setApplicationReturnValue (exitCode);
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
    void addCommands()
    {
        commands.addCommand ({ juce::String (Chars::doubleDash) + Id::help.toString(), {}, {}, {}, [this] (const juce::ArgumentList&) { printBannerAndHelp(); quit(); } });
        commands.addCommand ({ juce::String (Chars::doubleDash) + Id::version.toString(), {}, {}, {}, [this] (const juce::ArgumentList&) { writeVersion(); quit(); } });
        commands.addCommand ({ juce::String (Chars::doubleDash) + Id::build.toString(), {}, {}, {}, [this] (const juce::ArgumentList& arguments) { runBuild (arguments); } });
        commands.addCommand ({ juce::String (Chars::doubleDash) + Id::mcp.toString(), {}, {}, {}, [this] (const juce::ArgumentList& arguments) { runToolRelay (arguments); } });
        commands.addCommand ({ Id::daemon.toString(), {}, {}, {}, [this] (const juce::ArgumentList& arguments) { runDaemon (arguments); } });

        for (const auto& [word, scope] : map::Readiness {})
            commands.addCommand ({ word.toString(), {}, {}, {}, [this] (const juce::ArgumentList& arguments) { runOperation (arguments); } });

        commands.addDefaultCommand ({ {}, {}, {}, {}, [this] (const juce::ArgumentList& arguments) { runManifest (arguments); } });
    }

    void runBuild (const juce::ArgumentList& arguments)
    {
        arguments.checkMinNumArguments (commandIndex + 2);

        const auto folder { arguments[commandIndex + 1].resolveAsFile() };

        for (const auto& fileName : { files::core, files::syntaxTree, files::lsp, files::languageInterface })
        {
            const BinaryData::Raw resource { fileName };
            [[maybe_unused]] const auto isWritten { folder.getChildFile (fileName).replaceWithData (resource.data, static_cast<size_t> (resource.size)) };
            jassert (isWritten);
        }

        for (const auto& file : Processor::getOrCreateLanguages().getChildFile (files::languageCore).findChildFiles (juce::File::findFiles, false))
        {
            [[maybe_unused]] const auto isCopied { file.copyFileTo (folder.getChildFile (file.getFileName())) };
            jassert (isCopied);
        }

        juce::WaitableEvent finished;
        int exitCode { EXIT_FAILURE };
        jam::Subprocess subprocess;

        subprocess.launch (jam::Strings { Id::cast.toString(), files::languageSpell },
                           folder,
                           [&finished, &exitCode] (int processExitCode, const std::string&)
                           {
                               exitCode = processExitCode;
                               finished.signal();
                           },
                           [] (std::string_view chunk, bool)
                           {
                               fwrite (chunk.data(), sizeof (char), chunk.size(), stdout);
                               fflush (stdout);
                           });

        finished.wait();
        setApplicationReturnValue (exitCode);
        quit();
    }

    void runToolRelay (const juce::ArgumentList& arguments)
    {
        const auto manifestFile { getExistingManifestFile (arguments, manifestIndex) };
        const auto toolchainArgument { getToolchainArgument (arguments, manifestIndex) };

        toolRelay = std::make_unique<ToolRelay> (manifestFile, arguments[manifestIndex].resolveAsFile().getFileName(), toolchainArgument);
        toolRelay->start();
    }

    void runManifest (const juce::ArgumentList& arguments)
    {
        if (arguments.size() == 0)
        {
            printBannerAndHelp();
            quit();
        }
        else
        {
            const auto manifestFile { getExistingManifestFile (arguments, commandIndex) };
            const auto toolchainArgument { getToolchainArgument (arguments, commandIndex) };
            const int expectedCount { commandIndex + 1 + (toolchainArgument.isNotEmpty() ? 1 : 0) };

            if (arguments.size() == expectedCount)
            {
#if LING_USING_SCHEMA
                const auto document { jam::ConfigDocument::parse (manifestFile.loadFileAsString(), manifestFile.getFullPathName()) };
                const auto folder { manifestFile.getParentDirectory() };
                const auto result { document->getTables (Id::stdioClient).size() > 0
                                        ? StdioClientProbe::run (*document, folder)
                                        : SchemaSuite::run (folder.getChildFile (document->getValueTree (Id::manifest).getChildWithName (jam::Format::toType (Id::schema))[jam::Format::toType (Id::suite)].toString())) };

                profiler->drain();
                profiler->writeSamples (manifestFile, jam::Format::toType (juce::Identifier { ProjectInfo::projectName }));
                setApplicationReturnValue (result.wasOk() ? EXIT_SUCCESS : EXIT_FAILURE);
                quit();
#elif JAM_USING_PROFILER
                model.addListener (this);
                processor = std::make_unique<Processor> (model);
                manifest = std::make_unique<Manifest> (model);

                const auto document { jam::ConfigDocument::parse (manifestFile.loadFileAsString(), manifestFile.getFullPathName()) };
                const auto profilerTable { document->getValueTree (Id::manifest).getChildWithName (jam::Format::toType (Id::profiler)) };

                model.state.setProperty (Id::toolchainArgument, profilerTable[jam::Format::toType (Id::argument)].toString(), nullptr);
                model.state.setProperty (Id::manifest, manifestFile.getParentDirectory().getChildFile (profilerTable[jam::Format::toType (Id::manifest)].toString()).getFullPathName(), nullptr);
#else
                relay = std::make_unique<Relay> (manifestFile, toolchainArgument);
                relay->start();
#endif
            }
            else
            {
                juce::ConsoleApplication::fail (getErrorLine (jam::Strings::getView (arguments[expectedCount].text), text::Diagnostics::failFlagUnknown), EXIT_FAILURE);
            }
        }
    }

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
    void runDaemon (const juce::ArgumentList& arguments)
    {
        const auto manifestFile { getExistingManifestFile (arguments, manifestIndex) };
        const auto toolchainArgument { getToolchainArgument (arguments, manifestIndex) };

        lock = std::make_unique<juce::InterProcessLock> (juce::String (LINGModel::getDaemonName (manifestFile, jam::Strings::getView (toolchainArgument))));

        if (lock->enter (0))
        {
            processor = std::make_unique<Processor> (model);
            manifest = std::make_unique<Manifest> (model);
            server = std::make_unique<Server>();
            model.state.setProperty (Id::toolchainArgument, toolchainArgument, nullptr);
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
    void runOperation (const juce::ArgumentList& arguments)
    {
        const auto manifestFile { getExistingManifestFile (arguments, manifestIndex) };
        const auto toolchainArgument { getToolchainArgument (arguments, manifestIndex) };
        const int index { manifestIndex + 1 + (toolchainArgument.isNotEmpty() ? 1 : 0) };
        juce::Array<juce::var> requestArguments;
        juce::ValueTree request { jam::Format::toType (Id::request) };

        for (int argumentIndex { index }; argumentIndex < arguments.size(); ++argumentIndex)
            requestArguments.add (arguments[argumentIndex].text);

        request.setProperty (Id::command, arguments[commandIndex].text, nullptr);
        request.setProperty (Id::directory, juce::File::getCurrentWorkingDirectory().getFullPathName(), nullptr);
        request.setProperty (Id::positionEncoding, jam::Strings::fromView (lsp::PositionEncodingType::UTF16), nullptr);
        request.setProperty (Id::arguments, requestArguments, nullptr);

        juce::MemoryBlock requestData;
        juce::MemoryOutputStream stream { requestData, false };

        request.writeToStream (stream);
        stream.flush();

        operation = std::make_unique<Operation> (manifestFile, toolchainArgument, std::move (requestData));
        operation->start();
    }

#if JAM_USING_PROFILER
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

            processor->applyToDocuments ([&rows] (std::string_view path, const jam::Document& document, const Index& index) { appendDocument (rows, path, document, index); });
            jam::Format::appendDecimal (rows, static_cast<int> (model.state[Id::documents]));
            jam::Format::appendUtf8 (rows, Chars::space);
            rows.append (jam::Strings::getView (Id::documents.toString()));
            jam::Format::appendUtf8 (rows, Chars::newline);
            fwrite (rows.data(), sizeof (char), rows.size(), stdout);
            profiler->drain();
            profiler->writeSamples (getManifestFile ({ ProjectInfo::projectName, getCommandLineParameterArray() }, commandIndex), jam::Format::toType (juce::Identifier { ProjectInfo::projectName }));
            quit();
        }
    }
#endif

#if JAM_USING_PROFILER
    jam::SharedInstance<jam::Profiler> profiler { std::in_place, jam::Profiler::defaultCapacity };
#endif
    /** @brief The state model of the run. The application owns it. */
    LINGModel model;
    juce::ConsoleApplication commands;
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
    /** @brief The operation run. It sends the request to the daemon and prints the reply. */
    std::unique_ptr<Operation> operation;
    std::unique_ptr<Relay> relay;
    std::unique_ptr<ToolRelay> toolRelay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Application)
};

START_JUCE_APPLICATION (Application)
