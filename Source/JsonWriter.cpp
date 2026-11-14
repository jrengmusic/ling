#include "JsonWriter.h"

void JsonWriter::appendElement (std::string& json, std::string_view path, const jam::LanguageDocument& document, const jam::Document::Element& element)
{
    appendElement (json, path, document, element, [] (std::string&) {});
}

void JsonWriter::appendComment (std::string& json, const jam::LanguageDocument& document, const jam::Document::Element& element)
{
    if (const auto* tokens { element.find<jam::Document::Tokens> (Id::comment) }; tokens != nullptr)
    {
        jam::JsonWriter::appendKey (json, Id::comment);
        jam::JsonWriter::appendString (json, document.getView (jam::Document::Token (tokens->first().span.getStart(), tokens->last().span.getEnd(), map::SyntaxTokenType::comment)));
    }
}

void JsonWriter::appendSnippet (std::string& json, const jam::LanguageDocument& document, const jam::Document::Element& element)
{
    const auto& name { element.get<jam::Document::Tokens> (Id::name)->last() };
    const auto& parameters { *element.get<jam::Document::Tokens> (Id::parameters) };

    jam::JsonWriter::appendKey (json, Id::snippet);
    jam::Format::appendUtf8 (json, Chars::doubleQuote);
    jam::JsonWriter::appendStringBody (json, document.getView (name));
    jam::Format::appendUtf8 (json, Chars::openParen);

    for (int index { 0 }; index < parameters.size(); ++index)
    {
        if (index > 0)
        {
            jam::Format::appendUtf8 (json, Chars::comma);
            jam::Format::appendUtf8 (json, Chars::space);
        }

        jam::Format::appendUtf8 (json, Chars::dollarSign);
        jam::Format::appendUtf8 (json, Chars::openBrace);
        jam::Format::appendDecimal (json, index + 1);
        jam::Format::appendUtf8 (json, Chars::colon);
        jam::JsonWriter::appendStringBody (json, document.getView (parameters.at (index)));
        jam::Format::appendUtf8 (json, Chars::closeBrace);
    }

    jam::Format::appendUtf8 (json, Chars::closeParen);
    jam::Format::appendUtf8 (json, Chars::doubleQuote);
}

void JsonWriter::appendParameters (std::string& json, const jam::LanguageDocument& document, const jam::Document::Element& declaration)
{
    for (const auto& parameter : *declaration.get<jam::Document::Tokens> (Id::parameters))
    {
        jam::JsonWriter::appendSeparator (json);
        jam::JsonWriter::appendString (json, document.getView (parameter));
    }
}
