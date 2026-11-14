#include "Server.h"

Server::Server()
{
    model.addListener (this);
}

Server::~Server()
{
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

void Server::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == Id::documents and tree == model.state)
    {
        getPortFile().getParentDirectory().createDirectory();
        getPortFile().replaceWithText (juce::String (getBoundPort()));
        startTimer (model.state[Id::timeout]);
        model.removeListener (this);
        state.addListener (this);
    }
}

void Server::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& childWhichHasBeenAdded)
{
    if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::request)))
        stopTimer();
}

void Server::valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree&, int)
{
    if ((parentTree == state or parentTree.getParent() == state)
        and std::none_of (state.begin(), state.end(), [] (const juce::ValueTree& connection) { return connection.getNumChildren() > 0; }))
        startTimer (model.state[Id::timeout]);
}

void Server::timerCallback()
{
    stop();

    [[maybe_unused]] const auto isDeleted { getPortFile().deleteFile() };

    stopTimer();
    juce::JUCEApplicationBase::quit();
}
