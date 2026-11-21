#pragma once
#include "Client.h"

class Operation : public Client,
                  private juce::ValueTree::Listener
{
public:
    Operation (const juce::File& newManifestFile, const juce::String& newToolchainArgument, juce::MemoryBlock&& newRequest);

    ~Operation() override;

private:
    void connectionMade() override;

    void messageReceived (const juce::MemoryBlock& reply) override;

    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    juce::MemoryBlock request;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Operation)
};
