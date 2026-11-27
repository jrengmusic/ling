#include "Server.h"

Server::Workspace::Workspace (Server& newOwner, const juce::String& newKey, const juce::File& manifestFile, const juce::String& toolchainArgument)
    : owner (newOwner),
      key (newKey),
      message (*model.getParameter<jam::ParameterText> (jam::Format::toType (Id::overlay), Id::message),
               [this] (const juce::var& text)
               {
                   const auto& messageText { text.toString() };
                   juce::MemoryBlock json;

                   jam::JsonWriter::appendObject (json, [&messageText] (juce::MemoryBlock& notification)
                   {
                       Query::appendJsonRpc (notification);

                       jam::JsonWriter::appendKey (notification, lsp::Member::method);
                       jam::JsonWriter::appendString (notification, lsp::Method::showMessage);

                       jam::JsonWriter::appendKey (notification, lsp::Member::params);
                       jam::JsonWriter::appendObject (notification, [&messageText] (juce::MemoryBlock& params)
                       {
                           jam::JsonWriter::appendKey (params, lsp::Member::messageType);
                           jam::Format::appendDecimal (params, lsp::MessageType::Error);

                           jam::JsonWriter::appendKey (params, lsp::Member::message);
                           jam::JsonWriter::appendString (params, jam::Strings::getView (messageText), juce::JSON::Encoding::utf8);
                       });
                   });

                   owner.broadcast (state, json);
               }),
      progress (*model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::pendingJobCount),
                [this] (const juce::var& pendingValue)
                {
                    const auto pendingJobs { static_cast<int> (pendingValue) };
                    const auto totalJobs { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::jobCount)->getValue() };
                    const auto progressType { pendingJobs == 0 ? lsp::WorkDoneProgressKind::End : pendingJobs == totalJobs ? lsp::WorkDoneProgressKind::Begin : lsp::WorkDoneProgressKind::Report };

                    owner.broadcast (state, getProgress (progressType, pendingJobs, totalJobs));
                })
{
    model.addListener (this);
    model.state.setProperty (Id::toolchainArgument, toolchainArgument, nullptr);
    model.state.setProperty (Id::manifest, manifestFile.getFullPathName(), nullptr);
}

Server::Workspace::~Workspace()
{
    model.removeListener (this);
}

bool Server::Workspace::hasConnection (const juce::ValueTree& connectionState) const
{
    return connectionState.getParent() == state;
}

void Server::Workspace::addConnection (Connection& connection, juce::ValueTree& connectionState)
{
    stopTimer();
    state.appendChild (connectionState, nullptr);

    if (hasEmptyFiles())
    {
        connection.sendMessage (getNoPortNotification());
    }
    else
    {
        const auto pendingJobs { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::pendingJobCount)->getValue() };
        const auto totalJobs { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::jobCount)->getValue() };

        if (pendingJobs > 0)
            connection.sendMessage (getProgress (lsp::WorkDoneProgressKind::Begin, pendingJobs, totalJobs));
    }
}

void Server::Workspace::removeConnection (juce::ValueTree& connectionState)
{
    if (connectionState.getParent() == state)
        state.removeChild (connectionState, nullptr);

    if (state.getNumChildren() == 0)
        startTimer (model.state[Id::timeout]);
}

bool Server::Workspace::hasEmptyFiles() const
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    return files.isValid() and files.hasProperty (Id::buildFiles) and files.getNumChildren() == 0;
}

void Server::Workspace::valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
{
    if (parentTree == model.state and childWhichHasBeenAdded.hasType (jam::Format::toType (Id::files)))
    {
        if (childWhichHasBeenAdded.getNumChildren() == 0)
            owner.broadcast (state, getNoPortNotification());

        if (state.getNumChildren() == 0)
            startTimer (model.state[Id::timeout]);
    }
}

void Server::Workspace::timerCallback()
{
    const auto workspaceKey { key };

    owner.removeWorkspace (workspaceKey);
}

Server::Server()
{
    watcher.addFolder (juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory());
    watcher.coalesceEvents (LINGModel::coalesceMs);
    watcher.addListener (this);
}

Server::~Server()
{
    watcher.removeListener (this);
    stop();
}

void Server::start()
{
    [[maybe_unused]] const auto isListening { beginWaitingForSocket (anyPort, juce::IPAddress::local().toString()) };
    jassert (isListening);

    const auto portFile { getPortFile() };

    portFile.getParentDirectory().createDirectory();
    portFile.replaceWithText (juce::String (getBoundPort()));
    startTimer (LINGModel::defaultTimeoutMs);
}

juce::File Server::getPortFile()
{
    return LINGModel::getCacheDirectory().getChildFile (jam::Format::toFileName (LINGModel::getDaemonName().c_str(), files::lingPort));
}

juce::InterprocessConnection* Server::createConnectionObject()
{
    const auto connection { std::make_shared<std::unique_ptr<Connection>> (std::make_unique<Connection> (
        connections,
        [this] (Connection& clientConnection, juce::ValueTree& connectionState, const juce::File& manifestFile, const juce::String& toolchainArgument)
        {
            attachConnection (clientConnection, connectionState, manifestFile, toolchainArgument);
        },
        [this] (Connection&, juce::ValueTree& connectionState)
        {
            detachConnection (connectionState);
        })) };
    auto* object { connection->get() };

    juce::MessageManager::callAsync ([this, connection] { connections.add (std::move (*connection)); });
    return object;
}

void Server::attachConnection (Connection& connection, juce::ValueTree& connectionState, const juce::File& manifestFile, const juce::String& toolchainArgument)
{
    stopTimer();

    const auto key { jam::Strings::fromView (LINGModel::getProjectName (manifestFile, jam::Strings::getView (toolchainArgument))) };
    const auto [entry, isAdded] { workspaces.try_emplace (key) };
    auto& [workspaceKey, workspace] { *entry };

    if (isAdded)
        workspace = std::make_unique<Workspace> (*this, key, manifestFile, toolchainArgument);

    workspace->addConnection (connection, connectionState);
}

void Server::detachConnection (juce::ValueTree& connectionState)
{
    for (const auto& [workspaceKey, workspace] : workspaces)
    {
        if (workspace->hasConnection (connectionState))
        {
            workspace->removeConnection (connectionState);
            return;
        }
    }
}

void Server::removeWorkspace (const juce::String& key)
{
    workspaces.erase (key);

    if (workspaces.empty())
    {
        if (connections.isEmpty())
            timerCallback();
        else
            startTimer (LINGModel::defaultTimeoutMs);
    }
}

void Server::broadcast (const juce::ValueTree& workspaceConnections, const juce::MemoryBlock& message)
{
    for (const auto& connection : connections)
        if (connection->isIn (workspaceConnections))
            connection->sendMessage (message);
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

juce::MemoryBlock Server::getNoPortNotification()
{
    juce::MemoryBlock json;

    jam::JsonWriter::appendObject (json, [] (juce::MemoryBlock& notification)
    {
        Query::appendJsonRpc (notification);

        jam::JsonWriter::appendKey (notification, lsp::Member::method);
        jam::JsonWriter::appendString (notification, noPortMethod);
    });

    return json;
}

juce::MemoryBlock Server::getProgress (std::string_view progressType, int pendingJobs, int totalJobs)
{
    static constexpr int percentageScale { 100 };

    static const auto progressValues ([]
    {
        jam::Function::Map<std::string_view, jam::Function::pointerCapacity, void (juce::MemoryBlock&, int, int)> progressValues;

        progressValues.add (lsp::WorkDoneProgressKind::Begin, [] (const std::string_view&, juce::MemoryBlock& value, int doneJobs, int totalJobs)
        {
            jam::JsonWriter::appendKey (value, lsp::Member::title);
            jam::JsonWriter::appendString (value, text::Progress::progressTitle);

            jam::JsonWriter::appendKey (value, lsp::Member::percentage);
            jam::Format::appendDecimal (value, doneJobs * percentageScale / totalJobs);
        });

        progressValues.add (lsp::WorkDoneProgressKind::Report, [] (const std::string_view&, juce::MemoryBlock& value, int doneJobs, int totalJobs)
        {
            jam::JsonWriter::appendKey (value, lsp::Member::percentage);
            jam::Format::appendDecimal (value, doneJobs * percentageScale / totalJobs);

            jam::JsonWriter::appendKey (value, lsp::Member::message);
            jam::JsonWriter::appendString (value, jam::Strings::getView (juce::String { doneJobs } + text::Progress::progressSeparator + juce::String { totalJobs }));
        });

        progressValues.add (lsp::WorkDoneProgressKind::End, [] (const std::string_view&, juce::MemoryBlock&, int, int) {});

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

                progressValues.get (progressType, value, doneJobs, totalJobs);
            });
        });
    });

    return json;
}
