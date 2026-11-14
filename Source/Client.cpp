#include "Client.h"

Client::Client (const juce::File& newManifestFile, const juce::String& newToolchainArgument, const juce::String& newRequest)
    : juce::InterprocessConnection (true)
    , manifestFile (newManifestFile)
    , toolchainArgument (newToolchainArgument)
    , request (newRequest)
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
    disconnect();
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
    const auto port { Server::getPortFile (manifestFile, jam::Strings::getView (toolchainArgument)).loadFileAsString().getIntValue() };

    const auto isConnected { connectToSocket (juce::IPAddress::local().toString(), port, connectTimeoutMs) };

    if (isConnected)
        watcher.removeListener (this);

    return isConnected;
}

void Client::fileChanged (const juce::File& file, jam::File::Watcher::Event)
{
    if (file == Server::getPortFile (manifestFile, jam::Strings::getView (toolchainArgument)))
    {
        [[maybe_unused]] const auto isConnected { connectToDaemon() };
    }
}

void Client::connectionMade()
{
    sendMessage (juce::MemoryBlock { request.toRawUTF8(), request.getNumBytesAsUTF8() });
}

void Client::connectionLost()
{
    model.setMessage (text::Diagnostics::failDaemonLost);
    juce::JUCEApplicationBase::getInstance()->setApplicationReturnValue (EXIT_FAILURE);
    juce::JUCEApplicationBase::quit();
}

void Client::messageReceived (const juce::MemoryBlock& reply)
{
    if (reply[0] == static_cast<char> (Chars::doubleQuote))
    {
        if (const auto document { jam::JsonDocument::parse (std::string_view { static_cast<const char*> (reply.getData()), reply.getSize() }) }; document != nullptr)
        {
            model.setMessage (jam::Strings::fromView (document->getString (document->getValue())));
        }
    }
    else
    {
        fwrite (reply.getData(), sizeof (char), reply.getSize(), stdout);
        disconnect (-1, juce::InterprocessConnection::Notify::no);
        juce::JUCEApplicationBase::quit();
    }
}
