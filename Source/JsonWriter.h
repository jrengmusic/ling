#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

/**
 * @struct JsonWriter
 * @brief Appends JSON text to a string that the caller owns.
 *
 * The struct holds no state. Each function writes into the string that the
 * caller passes. appendSeparator places each JSON separator, so no caller places one.
 */
struct JsonWriter
{
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

        jam::JsonWriter::appendSeparator (json);
        jam::JsonWriter::appendObject (json, [&path, &document, &element, &token, &appendMembers] (std::string& object)
        {
            jam::JsonWriter::appendKey (object, Id::path);
            jam::JsonWriter::appendString (object, path);

            jam::JsonWriter::appendKey (object, Id::line);
            jam::Format::appendDecimal (object, static_cast<int> (document.getLineNumber (token.offset())));

            jam::JsonWriter::appendKey (object, Id::column);
            jam::Format::appendDecimal (object, static_cast<int> (document.getColumnNumber (token.offset())));

            jam::JsonWriter::appendKey (object, Id::type);
            jam::Format::appendDecimal (object, *element.get<int> (Id::type));

            jam::JsonWriter::appendKey (object, Id::name);
            jam::JsonWriter::appendString (object, document.getView (token));

            appendMembers (object);
        });
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

    /**
     * @brief Appends the comment key: the source text from the first to the last comment token of the element.
     *
     * The text keeps its comment markers. An element with no Id::comment gets no key.
     *
     * @param json      The JSON text that receives the key.
     * @param document  The document whose source holds the comment tokens.
     * @param element   The element whose Id::comment is written.
     */
    static void appendComment (std::string& json, const jam::CodeDocument& document, const jam::Document::Element& element);

    /**
     * @brief Appends the snippet key of a function: its name, then one placeholder for each parameter.
     *
     * A placeholder holds the text of its parameter token. That is the name of a
     * named parameter, or the source text of an unnamed parameter. A function with
     * no parameters gives an empty pair of parentheses. The value is a JSON string.
     * Its text is not escaped for the snippet grammar.
     *
     * @param json      The JSON text that receives the key.
     * @param document  The document whose source gives the text of the name and the parameters.
     * @param element   The function element. It must hold Id::name and Id::parameters.
     */
    static void appendSnippet (std::string& json, const jam::CodeDocument& document, const jam::Document::Element& element);
};
