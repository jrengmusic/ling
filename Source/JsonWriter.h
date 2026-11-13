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
            jam::JsonWriter::appendString (object, std::string_view (document.getSource()).substr (token.offset(), token.length()));

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
};
