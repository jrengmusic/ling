/**
 * @file ToolRelay.h
 * @brief Relays MCP tool calls between the standard input server and the daemon connection of a manifest.
 */

#pragma once
#include "ToolClient.h"
#include "Query.h"

/**
 * @class ToolRelay
 * @brief Serves MCP tool calls over standard input, with one ToolClient for each manifest.
 *
 * The clients are keyed by the full path name of their manifest. The relay exposes the declared tools through an MCP server and translates each
 * call into a request row. It queues cancel rows for cancelled requests. Each
 * daemon reply goes back to the MCP caller as a tool result or a tool error.
 * Daemon showMessage notifications update the model message.
 */
class ToolRelay
{
public:
    /**
     * @brief Creates the tool relay for a manifest and binds the MCP server handlers.
     *
     * Registers each query tool on @c server. Sets @c onInputClosed to quit with
     * success and @c onCancelled to queue an Id::cancel row on each client.
     *
     * @param newManifestFile       The default manifest. start() creates its client. getManifestFile() returns it for a query, or when no manifest is found.
     * @param newManifestName       The file name that a request manifest must match.
     * @param newToolchainArgument  The toolchain argument that the clients receive.
     */
    ToolRelay (const juce::File& newManifestFile, const juce::String& newManifestName, const juce::String& newToolchainArgument);

    /**
     * @brief Starts the daemon client of the manifest and opens the MCP standard input server.
     *
     * Starts the client before @c server so incoming tool calls queue onto an active client.
     */
    void start();

private:
    /**
     * @brief Builds an Id::request row for a tool call, and adds it to the client of the manifest that getManifestFile() returns for it.
     *
     * Sets Id::positionEncoding to lsp::PositionEncodingType::UTF16. Stores the current working
     * directory in Id::directory, so the daemon resolves relative file paths against it.
     *
     * @param requestId  The JSON-RPC request identifier from the MCP call.
     * @param document   The parsed JSON document holding the tool arguments.
     * @param arguments  The arguments object element of the tool call.
     * @param command    The ling operation identifier for the tool.
     * @param shape      The argument shape identifier from Query::getArgumentShape().
     */
    void addRequest (const juce::String& requestId, const jam::JsonDocument& document, const jam::Document::Element& arguments, const juce::Identifier& command, const juce::Identifier& shape);

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

    /**
     * @brief Returns the manifest of a request: the manifest found upward from the folder of its file, or manifestFile.
     *
     * A query request has no file, so it always gets manifestFile. For any other shape, the parent
     * folder of the request file is searched by LINGModel::getManifestFile() for manifestName. When
     * no manifest is found, manifestFile is returned.
     *
     * @param request  The request row. Its directory and its file argument give the file.
     * @param shape    The argument shape identifier from Query::getArgumentShape().
     * @return The manifest file of the request.
     */
    juce::File getManifestFile (const juce::ValueTree& request, const juce::Identifier& shape) const;

    /**
     * @brief Returns the client of a manifest, and creates and starts it when no client exists for the manifest path.
     *
     * @param requestManifest  The manifest that the client serves.
     * @return The client of the manifest.
     */
    ToolClient& getOrCreateClient (const juce::File& requestManifest);

    /** @brief The default manifest. start() creates its client, and getManifestFile() returns it when no other manifest applies. */
    const juce::File manifestFile;

    /** @brief The file name that a request manifest must have. getManifestFile() searches upward for it. */
    const juce::String manifestName;

    /** @brief The toolchain argument that each client receives. */
    const juce::String toolchainArgument;

    /** @brief MCP server exposing the tools from @c files::lingTools over standard input and output. */
    jam::ModelContextServer server { BinaryData::getString (files::lingTools).toStdString(), ProjectInfo::projectName, ProjectInfo::versionString };

    /** @brief One ToolClient for each manifest, keyed by the full path name of the manifest. */
    jam::HashMap<juce::String, std::unique_ptr<ToolClient>> clients;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolRelay)
};
