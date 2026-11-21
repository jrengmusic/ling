/**
 * @file Relay.h
 * @brief Relays LSP messages between the standard input and the daemon connection of a manifest.
 */

#pragma once
#include "Client.h"
#include "Query.h"

/**
 * @class Relay
 * @brief Serves the messages of the standard input for one manifest.
 *
 * The relay answers the lifecycle methods itself. A method in the operations table becomes a request row,
 * which the daemon connection sends. Any other method that carries an id gets an error reply.
 * Each daemon reply goes to the standard output as one frame.
 */
class Relay : public Client,
              private juce::ValueTree::Listener
{
public:
    /**
     * @brief Creates the relay for a manifest, and sets the standard input callback.
     *
     * The callback adds one to numMessages when the read result is zero or more.
     * It sets isOpen to 0 when the read result is negative, which marks the end of input.
     * The constructor also registers the lifecycle methods, and listens to the messages and the requests.
     *
     * @param newManifestFile       The manifest that the Client base receives.
     * @param newToolchainArgument  The toolchain argument that the Client base receives.
     */
    Relay (const juce::File& newManifestFile, const juce::String& newToolchainArgument);

    /** @brief Removes the relay as a listener of the messages and the requests, and disconnects. */
    ~Relay() override;

    /**
     * @brief Starts the client, and opens the standard input with bufferSize and maximumPeriodMilliseconds.
     *
     * The open call must succeed (jassert).
     */
    void start();

private:
    /**
     * @brief Sends each queued request row to the daemon, when the connection is made.
     *
     * A row leaves the requests when it is sent.
     */
    void connectionMade() override;

    /**
     * @brief Writes a daemon reply to the standard output as one frame.
     *
     * @param reply  The reply bytes. They form the body of the frame.
     */
    void messageReceived (const juce::MemoryBlock& reply) override;

    /**
     * @brief Sends a child that is added to the requests, when the connection is connected.
     *
     * A child added to any other tree does nothing.
     *
     * @param parentTree              The tree that gained the child.
     * @param childWhichHasBeenAdded  The added child.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Applies the complete frames of the standard input when numMessages changes, and exits when isOpen is false.
     *
     * Each frame body is parsed with JsonDocument::read, which borrows the bytes. The parse must succeed (jassert).
     * The ring keeps the bytes until the whole callback returns.
     *
     * @param tree      The tree that changed. Only the messages tree acts.
     * @param property  The property that changed.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Dispatches one parsed message to a lifecycle method, an operation, or an error reply.
     *
     * @param document  The parsed message. Its root holds the method name and the id.
     */
    void setMessage (const jam::JsonDocument& document);

    /**
     * @brief Appends a request row to the requests, when the server is initialized and not shut down.
     *
     * A message after shutdown gets an InvalidRequest error reply.
     * A message before initialization gets a ServerNotInitialized error reply.
     *
     * @param document   The parsed message.
     * @param root       The root element of the message.
     * @param idToken    The id of the message. The row and the error reply both carry it.
     * @param operation  The command name that the row carries.
     */
    void addRequest (const jam::JsonDocument& document, const jam::Document::Element& root, const jam::Document::Token& idToken, const juce::Identifier& operation);

    /**
     * @brief Sends a request row to the daemon connection, and removes the row from the requests.
     *
     * The row is written to a memory block as a tree, and the block is sent as one message.
     *
     * @param row  The request row to send.
     */
    void sendRequest (const juce::ValueTree& row);

    void sendEmptyResult (const juce::ValueTree& row);

    /**
     * @brief Writes a response to the standard output as one frame.
     *
     * The response holds the members from Query::appendJsonRpc, the id of the message, and one member.
     * The member name is a name such as result or error.
     *
     * @tparam Function     Callable with the signature `void (std::string& members)`.
     * @param document      The parsed message that holds the id.
     * @param idToken       The id token that the response copies.
     * @param member        The name of the member that appendValue fills.
     * @param appendValue   Appends the value of the member to the members text.
     */
    template <typename Function>
    void sendResponse (std::string_view id, std::string_view member, const Function& appendValue)
    {
        std::string json;

        jam::JsonWriter::appendObject (json, [id, member, &appendValue] (std::string& members)
        {
            Query::appendJsonRpc (members);
            jam::JsonWriter::appendKey (members, lsp::Member::id);
            jam::Format::appendBytes (members, id);
            jam::JsonWriter::appendKey (members, member);
            appendValue (members);
        });

        jam::StdioProtocol::writeMessage (stdio, json);
    }

    /**
     * @brief Sends an error reply for a message, with the method name as the error message.
     *
     * @param document  The parsed message.
     * @param idToken   The id of the message.
     * @param code      The error code of the reply.
     */
    void sendError (const jam::JsonDocument& document, const jam::Document::Token& idToken, int code);

    /**
     * @brief Answers the initialize request, and marks the messages tree as initialized.
     *
     * The reply holds the capabilities and the server info. The stored position encoding comes from the params.
     *
     * @param document  The parsed message.
     * @param root      The root element of the message.
     * @param idToken   The id token of the request. It must not be null (jassert).
     */
    void initialize (const jam::JsonDocument& document, const jam::Document::Element& root, const jam::Document::Token& idToken);

    /** @brief Queues a connection row for the daemon, with the stored position encoding. */
    void initialized();

    void cancelRequest (const jam::JsonDocument& document, const jam::Document::Element& root);

    void didOpen (const jam::JsonDocument& document, const jam::Document::Element& root);

    void didChange (const jam::JsonDocument& document, const jam::Document::Element& root);

    void didClose (const jam::JsonDocument& document, const jam::Document::Element& root);

    juce::ValueTree getRequest (const jam::JsonDocument& document, const jam::Document::Element& root, const juce::Identifier& operation) const;

    /**
     * @brief Marks the relay as shut down, and replies with a null result.
     *
     * @param document  The parsed message.
     * @param idToken   The id token of the request. It must not be null (jassert).
     */
    void shutdown (const jam::JsonDocument& document, const jam::Document::Token& idToken);

    /**
     * @brief Sets the application exit code, and quits the application.
     *
     * The exit code is success when shutdown was received. Otherwise it is failure.
     */
    void exit();

    /**
     * @brief Returns the position encoding that the client accepts.
     *
     * The result is the UTF8 value when the client lists it in its general position encodings.
     * Otherwise the result is the UTF16 value.
     *
     * @param document  The parsed message.
     * @param root      The root element of the message.
     * @return The encoding name from lsp::PositionEncodingType.
     */
    static std::string_view getPositionEncoding (const jam::JsonDocument& document, const jam::Document::Element& root);

    /**
     * @brief Returns the local file that the uri of the request names.
     *
     * The uri comes from the text document of the params. When no text document is present, it comes from the item.
     *
     * @param document  The parsed message.
     * @param params    The params element of the request.
     * @return The local file of the uri.
     */
    static juce::File getFile (const jam::JsonDocument& document, const jam::Document::Element& params);

    /**
     * @brief Returns the position element of the request.
     *
     * The position of the params is used when present. Otherwise the start of the item selection range is used.
     *
     * @param document  The parsed message.
     * @param params    The params element of the request.
     * @return The position element.
     */
    static const jam::Document::Element& getPosition (const jam::JsonDocument& document, const jam::Document::Element& params);

    /**
     * @brief Returns the arguments of a request row, from the params of the message.
     *
     * A query gives one string argument. Otherwise the arguments hold the full path of the file.
     * When a position or an item is present, they also hold the line and the character.
     * Each line and character number is one more than the value in the message.
     *
     * @param document  The parsed message.
     * @param params    The params element of the request.
     * @return The arguments of the row.
     */
    static juce::Array<juce::var> getRequestArguments (const jam::JsonDocument& document, const jam::Document::Element& params, const juce::Identifier& operation);

    /** @brief The messages tree of the model. It holds the standard input counters and the connection state. */
    juce::ValueTree messages { model.state.getOrCreateChildWithName (jam::Format::toType (Id::messages), nullptr) };

    /** @brief The request rows that wait for the daemon connection. */
    juce::ValueTree requests { model.state.getOrCreateChildWithName (jam::Format::toType (Id::requests), nullptr) };

    /** @brief The operations table. Each method name maps to the command name of its request row. */
    jam::SharedInstance<map::Operations> operations { std::in_place };

    /** @brief The lifecycle handlers, keyed by method name. Each handler takes the document, the root, and the id token. */
    jam::Function::Map<juce::Identifier, jam::Function::pointerCapacity, void (const jam::JsonDocument&, const jam::Document::Element&, const jam::Document::Token&)> requestMethods;

    jam::Function::Map<juce::Identifier, jam::Function::pointerCapacity, void (const jam::JsonDocument&, const jam::Document::Element&)> notificationMethods;

    /** @brief Attaches the numMessages property of the messages tree to a parameter with initial value zero. The callback calls model.flush(). */
    jam::Model::ParameterAttachment numMessagesAttachment { model.createAndAddParameter<jam::Parameter<int>> (messages, Id::numMessages, 0), [this] (const juce::var&) { model.flush(); } };

    /** @brief Attaches the isOpen property of the messages tree to a parameter with initial value one. The callback calls model.flush(). */
    jam::Model::ParameterAttachment isOpenAttachment { model.createAndAddParameter<jam::Parameter<int>> (messages, Id::isOpen, 1), [this] (const juce::var&) { model.flush(); } };

    /** @brief Size in bytes of each standard input and standard output ring. */
    static constexpr int bufferSize { 1 << 20 };

    /** @brief Longest period, in milliseconds, of the standard input readiness clock. */
    static constexpr double maximumPeriodMilliseconds { 0.0 };

    /** @brief The standard input and the standard output of the process. */
    jam::StdioServer stdio;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Relay)
};
