#include "Server.h"

Server::Server()
{
    watcher.addFolder (juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory());
    watcher.coalesceEvents (LINGModel::coalesceMs);
    watcher.addListener (this);
    model.addListener (this);
}

Server::~Server()
{
    watcher.removeListener (this);
    stop();
    model.removeListener (this);
    state.removeListener (this);
}

void Server::start()
{
    [[maybe_unused]] const auto isListening { beginWaitingForSocket (anyPort, juce::IPAddress::local().toString()) };
    jassert (isListening);
}

juce::File Server::getPortFile (const juce::File& manifest, std::string_view toolchainArgument)
{
    return LINGModel::getCacheDirectory (manifest, toolchainArgument).getChildFile (jam::Format::toFileName (LINGModel::getDaemonName (manifest, toolchainArgument).c_str(), files::lingPort));
}

juce::File Server::getPortFile() const
{
    const auto toolchainArgument { model.state[Id::toolchainArgument].toString() };

    return getPortFile (juce::File { model.state[Id::manifest].toString() }, jam::Strings::getView (toolchainArgument));
}

juce::InterprocessConnection* Server::createConnectionObject()
{
    const auto connection { std::make_shared<std::unique_ptr<Connection>> (std::make_unique<Connection> (connections)) };
    auto* object { connection->get() };

    juce::MessageManager::callAsync ([this, connection] { connections.add (std::move (*connection)); });
    return object;
}

void Server::valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
{
    if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::files)) and parentTree == model.state)
    {
        getPortFile().getParentDirectory().createDirectory();

        if (childWhichHasBeenAdded.getNumChildren() == 0)
        {
            getPortFile().replaceWithText (juce::String (noPort));
            stop();
            juce::JUCEApplicationBase::quit();
        }
        else
        {
            getPortFile().replaceWithText (juce::String (getBoundPort()));
            if (state.getNumChildren() == 0)
                startTimer (model.state[Id::timeout]);

            model.removeListener (this);
            state.addListener (this);
        }
    }
    else if (parentTree == state)
    {
        stopTimer();

        const auto pendingJobs { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::pendingJobCount)->getValue() };
        const auto totalJobs { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::jobCount)->getValue() };

        if (pendingJobs > 0)
        {
            const auto json { getProgress (lsp::WorkDoneProgressKind::Begin, pendingJobs, totalJobs) };

            for (const auto& connection : connections)
                connection->sendMessage (json);
        }
    }
}

void Server::valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree&, int)
{
    if (parentTree == state and state.getNumChildren() == 0)
        startTimer (model.state[Id::timeout]);
}

void Server::timerCallback()
{
    stop();

    [[maybe_unused]] const auto isDeleted { getPortFile().deleteFile() };

    stopTimer();
    juce::JUCEApplicationBase::quit();
}

void Server::fileChanged (const juce::File& file, jam::File::Watcher::Event)
{
    if (file == juce::File::getSpecialLocation (juce::File::currentExecutableFile))
        timerCallback();
}

juce::MemoryBlock Server::getProgress (std::string_view progressType, int pendingJobs, int totalJobs)
{
    static constexpr int percentageScale { 100 };

    static const auto progressValues ([]
    {
        jam::Function::Map<juce::Identifier, jam::Function::pointerCapacity, void (juce::MemoryBlock&, int, int)> progressValues;

        progressValues.add (juce::Identifier { jam::Strings::fromView (lsp::WorkDoneProgressKind::Begin) }, [] (const juce::Identifier&, juce::MemoryBlock& value, int doneJobs, int totalJobs)
        {
            jam::JsonWriter::appendKey (value, lsp::Member::title);
            jam::JsonWriter::appendString (value, text::Progress::progressTitle);

            jam::JsonWriter::appendKey (value, lsp::Member::percentage);
            jam::Format::appendDecimal (value, doneJobs * percentageScale / totalJobs);
        });

        progressValues.add (juce::Identifier { jam::Strings::fromView (lsp::WorkDoneProgressKind::Report) }, [] (const juce::Identifier&, juce::MemoryBlock& value, int doneJobs, int totalJobs)
        {
            jam::JsonWriter::appendKey (value, lsp::Member::percentage);
            jam::Format::appendDecimal (value, doneJobs * percentageScale / totalJobs);

            jam::JsonWriter::appendKey (value, lsp::Member::message);
            jam::JsonWriter::appendString (value, jam::Strings::getView (juce::String { doneJobs } + text::Progress::progressSeparator + juce::String { totalJobs }));
        });

        progressValues.add (juce::Identifier { jam::Strings::fromView (lsp::WorkDoneProgressKind::End) }, [] (const juce::Identifier&, juce::MemoryBlock&, int, int) {});

        return progressValues;
    }());

    const auto doneJobs { totalJobs - pendingJobs };
    juce::MemoryBlock json;

    jam::JsonWriter::appendObject (json, [&progressType, doneJobs, totalJobs] (juce::MemoryBlock& notification)
    {
        Query::appendJsonRpc (notification);

        jam::JsonWriter::appendKey (notification, lsp::Member::method);
        jam::JsonWriter::appendString (notification, lsp::Method::progress);

        jam::JsonWriter::appendKey (notification, lsp::Member::params);
        jam::JsonWriter::appendObject (notification, [&progressType, doneJobs, totalJobs] (juce::MemoryBlock& params)
        {
            jam::JsonWriter::appendKey (params, lsp::Member::token);
            jam::JsonWriter::appendString (params, text::Progress::progressToken);

            jam::JsonWriter::appendKey (params, lsp::Member::value);
            jam::JsonWriter::appendObject (params, [&progressType, doneJobs, totalJobs] (juce::MemoryBlock& value)
            {
                jam::JsonWriter::appendKey (value, lsp::Member::type);
                jam::JsonWriter::appendString (value, progressType);

                progressValues.get (juce::Identifier { jam::Strings::fromView (progressType) }, value, doneJobs, totalJobs);
            });
        });
    });

    return json;
}
