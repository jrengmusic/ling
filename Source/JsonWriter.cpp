#include "JsonWriter.h"

void JsonWriter::appendElement (std::string& json, std::string_view path, const jam::CodeDocument& document, const jam::Document::Element& element)
{
    appendElement (json, path, document, element, [] (std::string&) {});
}
