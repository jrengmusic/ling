#include "SymbolDocument.h"

std::unique_ptr<SymbolDocument> SymbolDocument::read (std::string_view stored)
{
    auto document { std::make_unique<SymbolDocument>() };

    if (document->Document::read (emptySource, stored))
        return document;

    return nullptr;
}

int SymbolDocument::getToken (Cursor& cursor, int)
{
    return static_cast<int> (cursor.source.size() - cursor.getOffset());
}

void SymbolDocument::build()
{
}

const jam::Document::Vocabulary& SymbolDocument::getVocabulary() const
{
    return getPlainTextVocabulary();
}
