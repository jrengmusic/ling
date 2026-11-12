#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

/**
 * @struct JsonWriter
 * @brief Appends JSON text to a string that the caller owns.
 *
 * The struct holds no state. Each function writes into the string that the
 * caller passes. appendSeparator places each comma, so no caller places a comma.
 */
struct JsonWriter
{
    /**
     * @brief Appends a text as a JSON string, with its quotes and its escapes.
     *
     * @param json  The JSON text that receives the string.
     * @param text  The text to append. The JUCE JSON writer escapes its characters.
     */
    static void appendString (std::string& json, std::string_view text);

    /**
     * @brief Appends a separator, the key as a JSON string, and a colon.
     *
     * @param json  The JSON text that receives the key.
     * @param key   The key. Its text is the name of the member.
     */
    static void appendKey (std::string& json, const juce::Identifier& key);

    /**
     * @brief Appends a comma when the JSON text holds an entry before the current point.
     *
     * The last byte decides. An empty text, or a text that ends with an opening
     * bracket or an opening brace, has no entry before the point, so no comma is
     * appended.
     *
     * @param json  The JSON text that receives the comma.
     */
    static void appendSeparator (std::string& json);

    /**
     * @brief Appends an array: an opening bracket, the entries, and a closing bracket.
     *
     * @tparam Function      A callable type that takes (std::string&).
     * @param json           The JSON text that receives the array.
     * @param appendEntries  The callable that appends the entries to the same text.
     */
    template <typename Function>
    static void appendArray (std::string& json, const Function& appendEntries)
    {
        jam::Format::appendUtf8 (json, Chars::openBracket);
        appendEntries (json);
        jam::Format::appendUtf8 (json, Chars::closeBracket);
    }

    /**
     * @brief Appends one element object: its path, its position, its type, its name, and its extra members.
     *
     * The element must hold a name. The position is the line and the column of its
     * last name token. appendMembers runs before the closing brace, so each caller
     * adds its own members to the same object. Hover and document symbols need
     * this, because each adds members that the others do not.
     *
     * @tparam Function      A callable type that takes (std::string&).
     * @param json           The JSON text that receives the object.
     * @param path           The path text of the document that holds the element.
     * @param document       The document that holds the element. Its lines and its source give the position and the name.
     * @param element        The element to append. It must hold a name.
     * @param appendMembers  The callable that appends the extra members to the same object.
     */
    template <typename Function>
    static void appendElement (std::string& json, std::string_view path, const jam::CodeDocument& document, const jam::Document::Element& element, const Function& appendMembers)
    {
        const auto& token { element.get<jam::Document::Tokens> (Id::name)->last() };

        appendSeparator (json);
        jam::Format::appendUtf8 (json, Chars::openBrace);

        appendKey (json, Id::path);
        appendString (json, path);

        appendKey (json, Id::line);
        jam::Format::appendDecimal (json, static_cast<int> (document.getLineNumber (token.offset())));

        appendKey (json, Id::column);
        jam::Format::appendDecimal (json, static_cast<int> (document.getColumnNumber (token.offset())));

        appendKey (json, Id::type);
        jam::Format::appendDecimal (json, *element.get<int> (Id::type));

        appendKey (json, Id::name);
        appendString (json, std::string_view (document.getSource()).substr (token.offset(), token.length()));

        appendMembers (json);
        jam::Format::appendUtf8 (json, Chars::closeBrace);
    }

    /**
     * @brief Appends one element object with no extra members.
     *
     * @param json      The JSON text that receives the object.
     * @param path      The path text of the document that holds the element.
     * @param document  The document that holds the element.
     * @param element   The element to append. It must hold a name.
     */
    static void appendElement (std::string& json, std::string_view path, const jam::CodeDocument& document, const jam::Document::Element& element);
};
