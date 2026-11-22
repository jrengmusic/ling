/**
 * @file ToolRelay.h
 * @brief Relays MCP tool calls between the standard input server and the daemon connection of a manifest.
 */

#pragma once
#include "Client.h"
#include "Query.h"

/**
 * @class ToolRelay
 * @brief Serves MCP tool calls over standard input for one manifest.
 *
 * The relay exposes the declared tools through an MCP server and translates each
 * call into a request row. It queues cancel rows for cancelled requests. Each
 * daemon reply goes back to the MCP caller as a tool result or a tool error.
 * Daemon showMessage notifications update the model message.
 */
class ToolRelay : public Client,
                  private juce::ValueTree::Listener
{
public:
    /**
     * @brief Creates the tool relay for a manifest and binds the MCP server handlers.
     *
     * Registers each query tool on @c server. Sets @c onInputClosed to quit with
     * success and @c onCancelled to queue an Id::cancel row. Adds this relay as a
     * listener to @c model.state.
     *
     * @param newManifestFile       The manifest that the Client base receives.
     * @param newToolchainArgument  The toolchain argument that the Client base receives.
     */
    ToolRelay (const juce::File& newManifestFile, const juce::String& newToolchainArgument);

    /** @brief Removes the relay from the listeners of @c model.state and disconnects. */
    ~ToolRelay() override;

    /**
     * @brief Starts the daemon client connection and opens the MCP standard input server.
     *
     * Starts @c Client before @c server so incoming tool calls queue onto an active client.
     */
    void start();

private:
    /**
     * @brief Sends every queued row in @c requests to the daemon when the connection opens.
     *
     * Each row leaves @c requests as sendRequest() sends it.
     */
    void connectionMade() override;

    /**
     * @brief Routes a daemon JSON-RPC message to the model or the MCP server.
     *
     * Parses @p reply and asserts valid JSON. Forwards @c showMessage notifications to
     * @c model.setMessage(). Sends @c result payloads via @c server.sendResponse() and
     * @c error messages via @c server.sendToolError().
     *
     * @param reply  JSON-RPC message bytes received from the daemon.
     */
    void messageReceived (const juce::MemoryBlock& reply) override;

    /**
     * @brief Dispatches a child added to @c requests, or reports no daemon when no daemon port exists.
     *
     * Sends @p childWhichHasBeenAdded when connected, or calls sendNoDaemonError() when
     * @c model.state Id::port equals @c Server::noPort. Ignores other parent trees.
     *
     * @param parentTree              The tree that gained the child.
     * @param childWhichHasBeenAdded  The added request or cancel row.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Drains @c requests via sendNoDaemonError() when @c model.state Id::port becomes @c Server::noPort.
     *
     * Clears pending rows so MCP callers receive an immediate tool error when no daemon
     * serves the manifest.
     *
     * @param tree      The tree whose property changed.
     * @param property  The changed property identifier.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Builds an Id::request row for a tool call and appends it to @c requests.
     *
     * Sets UTF-16 position encoding to match the tool schema. Stores the current working
     * directory so the daemon resolves relative file paths.
     *
     * @param requestId  The JSON-RPC request identifier from the MCP call.
     * @param document   The parsed JSON document holding the tool arguments.
     * @param arguments  The validated arguments object element of the tool call.
     * @param command    The ling operation identifier for the tool.
     * @param shape      The argument shape identifier from Query::getArgumentShape().
     */
    void addRequest (const juce::String& requestId, const jam::JsonDocument& document, const jam::Document::Element& arguments, const juce::Identifier& command, const juce::Identifier& shape);

    /**
     * @brief Serializes a queued row, sends it to the daemon, and removes it from @c requests.
     *
     * @param row  The Id::request or Id::cancel row to send.
     */
    void sendRequest (const juce::ValueTree& row);

    /**
     * @brief Sends an `isError: true` tool error when @p row is an identified request, then removes @p row.
     *
     * Formats the error text from @c text::Diagnostics::failNoDaemon and @c manifestFile
     * via @c server.sendToolError().
     *
     * @param row  The queued row to drain from @c requests.
     */
    void sendNoDaemonError (const juce::ValueTree& row);

    /**
     * @brief Extracts the positional argument array of a request row from MCP tool arguments.
     *
     * Packs file, line, and column for Id::position. Packs file for Id::file, and
     * query for Id::query. Returns an empty array for any other shape.
     *
     * @param document   The parsed JSON document holding the tool arguments.
     * @param arguments  The validated arguments object element of the tool call.
     * @param shape      The argument shape identifier selecting which fields to extract.
     * @return Ordered argument values for the request row.
     */
    static juce::Array<juce::var> getRequestArguments (const jam::JsonDocument& document, const jam::Document::Element& arguments, const juce::Identifier& shape);

    /** @brief Queued Id::request and Id::cancel rows waiting for the daemon connection. */
    juce::ValueTree requests { model.state.getOrCreateChildWithName (jam::Format::toType (Id::requests), nullptr) };

    /** @brief MCP server exposing the tools from @c files::lingTools over standard input and output. */
    jam::ModelContextServer server { BinaryData::getString (files::lingTools).toStdString(), ProjectInfo::projectName, ProjectInfo::versionString };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolRelay)
};
