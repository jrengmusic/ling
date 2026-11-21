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
}

Client::~Client()
{
    watcher.removeListener (this);
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
        model.state.setProperty (Id::port, portFile.loadFileAsString().getIntValue(), nullptr);

        [[maybe_unused]] const auto isConnected { connectToDaemon() };
    }
}

void Client::connectionLost()
{
    model.setMessage (text::Diagnostics::failDaemonLost);
    juce::JUCEApplicationBase::getInstance()->setApplicationReturnValue (EXIT_FAILURE);
    juce::JUCEApplicationBase::quit();
}
