#include "Client.h"

Client::Client (const juce::File& newManifestFile, const juce::String& newToolchainArgument)
    : juce::InterprocessConnection (true)
    , manifestFile (newManifestFile)
    , toolchainArgument (newToolchainArgument)
{
    const auto portDirectory { Server::getPortFile (manifestFile, jam::Strings::getView (toolchainArgument)).getParentDirectory() };

    portDirectory.createDirectory();
    watcher.addFolder (portDirectory);
    watcher.coalesceEvents (LINGModel::coalesceMs);
    watcher.addListener (this);
    model.state.getChildWithName (jam::Format::toType (Id::connections)).appendChild (state, nullptr);
}

Client::~Client()
{
    watcher.removeListener (this);
    state.getParent().removeChild (state, nullptr);
}

void Client::start()
{
    if (not connectToDaemon())
    {
        jam::Strings daemonArguments { juce::File::getSpecialLocation (juce::File::currentExecutableFile).getFullPathName(), Id::daemon.toString(), manifestFile.getFullPathName() };

        if (toolchainArgument.isNotEmpty())
            daemonArguments.add (juce::String (Chars::doubleDash) + toolchainArgument);

        if (not jam::ChildProcess::startDetached (daemonArguments))
        {
            model.setMessage (text::Diagnostics::failDaemonStart);
            juce::JUCEApplicationBase::getInstance()->setApplicationReturnValue (EXIT_FAILURE);
            juce::JUCEApplicationBase::quit();
        }
    }
}

bool Client::connectToDaemon()
{
    static constexpr int connectTimeoutMs { 200 };

    const auto port { Server::getPortFile (manifestFile, jam::Strings::getView (toolchainArgument)).loadFileAsString().getIntValue() };
    const auto isConnected { connectToSocket (juce::IPAddress::local().toString(), port, connectTimeoutMs) };

    if (isConnected)
        watcher.removeListener (this);

    return isConnected;
}

void Client::fileChanged (const juce::File& file, jam::File::Watcher::Event)
{
    const auto portFile { Server::getPortFile (manifestFile, jam::Strings::getView (toolchainArgument)) };

    if (file == portFile)
    {
        state.setProperty (Id::port, portFile.loadFileAsString().getIntValue(), nullptr);

        [[maybe_unused]] const auto isConnected { connectToDaemon() };
    }
}

void Client::connectionLost()
{
    model.setMessage (text::Diagnostics::failDaemonLost);
    juce::JUCEApplicationBase::getInstance()->setApplicationReturnValue (EXIT_FAILURE);
    juce::JUCEApplicationBase::quit();
}

void Client::connectionMade()
{
    while (requests.getNumChildren() > 0)
    {
        const auto row { requests.getChild (0) };

        sendRequest (row);
    }
}

void Client::sendRequest (const juce::ValueTree& row)
{
    juce::MemoryBlock data;
    juce::MemoryOutputStream stream { data, false };

    row.writeToStream (stream);
    stream.flush();
    sendMessage (data);
    requests.removeChild (row, nullptr);
}

void Client::setMessage (const jam::JsonDocument& document, const jam::Document::Element& root)
{
    if (document.getString (document.get (root, lsp::Member::method)).compare (lsp::Method::showMessage) == 0)
    {
        const auto& params { *document.getChildByToken (root, document.get (root, lsp::Member::params)) };

        model.setMessage (jam::Strings::fromView (document.getString (document.get (params, lsp::Member::message))));
    }
}
