/**
 * @file ToolClient.h
 * @brief Daemon client of one manifest that returns the daemon replies to the MCP tool calls.
 */

#pragma once
#include "Client.h"

/**
 * @class ToolClient
 * @brief Daemon client of one manifest that sends MCP tool requests and returns each daemon reply to the MCP server.
 *
 * A row added to requests is sent at once while connected. Otherwise it waits for the connection,
 * or it receives a tool error when the manifest has no daemon port. Daemon result and error
 * messages go to the MCP server. Daemon showMessage notifications go to the model message.
 */
class ToolClient : public Client,
                   private juce::ValueTree::Listener
{
public:
    /**
     * @brief Creates the client of a manifest, and listens to state for changes of the daemon port and of the requests.
     *
     * @param newManifestFile       The manifest that identifies the daemon.
     * @param newToolchainArgument  The toolchain argument. Empty when none is given.
     * @param newServer             The MCP server that receives each result and tool error.
     */
    ToolClient (const juce::File& newManifestFile, const juce::String& newToolchainArgument, jam::ModelContextServer& newServer);

    /** @brief Removes the listener from state and disconnects from the daemon. */
    ~ToolClient() override;

    /**
     * @brief Appends a request or cancel row to requests, which starts valueTreeChildAdded() for the row.
     *
     * @param request  The request row or the cancel row to append.
     */
    void addRequest (const juce::ValueTree& request);

private:
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
     * @c state Id::port equals @c Server::noPort. Ignores other parent trees.
     *
     * @param parentTree              The tree that gained the child.
     * @param childWhichHasBeenAdded  The added request or cancel row.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Drains @c requests via sendNoDaemonError() when @c state Id::port becomes @c Server::noPort.
     *
     * Clears pending rows so MCP callers receive an immediate tool error when no daemon
     * serves the manifest.
     *
     * @param tree      The tree whose property changed.
     * @param property  The changed property identifier.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Sends an `isError: true` tool error when @p row is an identified request, then removes @p row.
     *
     * Formats the error text from @c text::Diagnostics::failNoDaemon and @c manifestFile
     * via @c server.sendToolError().
     *
     * @param row  The queued row to drain from @c requests.
     */
    void sendNoDaemonError (const juce::ValueTree& row);

    /** @brief The MCP server that receives each result and tool error of this client. */
    jam::ModelContextServer& server;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolClient)
};
