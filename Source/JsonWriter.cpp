#include "JsonWriter.h"

void JsonWriter::appendString (std::string& json, std::string_view text)
{
    const auto escaped { juce::JSON::escapeString (jam::Strings::fromView (text)) };

    jam::Format::appendUtf8 (json, Chars::doubleQuote);
    json.append (jam::Strings::getView (escaped));
    jam::Format::appendUtf8 (json, Chars::doubleQuote);
}

void JsonWriter::appendKey (std::string& json, const juce::Identifier& key)
{
    appendSeparator (json);
    appendString (json, jam::Strings::getView (key.toString()));
    jam::Format::appendUtf8 (json, Chars::colon);
}

void JsonWriter::appendSeparator (std::string& json)
{
    if (not json.empty() and json.back() != static_cast<char> (Chars::openBracket) and json.back() != static_cast<char> (Chars::openBrace))
        jam::Format::appendUtf8 (json, Chars::comma);
}

void JsonWriter::appendElement (std::string& json, std::string_view path, const jam::CodeDocument& document, const jam::Document::Element& element)
{
    appendElement (json, path, document, element, [] (std::string&) {});
}
