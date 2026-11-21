## languages

```
@brief The extension and the name of each built-in language.
```

+--------+------------+
| key    | value      |
+========+============+
| `md`   | `markdown` |
| `mmd`  | `mermaid`  |
| `json` | `json`     |
| `css`  | `css`      |
| `xml`  | `xml`      |
| `html` | `html`     |
+--------+------------+

## markdownElements

```
@brief The role and the symbol type of each markdown element that holds a name. A heading declares its text. A link refers to its destination.
```

+-----------------------------------------------------------+----------------------------------------------------------------------------------------------+
| key                                                       | value                                                                                        |
+===========================================================+==============================================================================================+
| map::HeadingTag::getInstance()->get (map::HeadingTag::h1) | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) |
| map::HeadingTag::getInstance()->get (map::HeadingTag::h2) | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) |
| map::HeadingTag::getInstance()->get (map::HeadingTag::h3) | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) |
| map::HeadingTag::getInstance()->get (map::HeadingTag::h4) | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) |
| map::HeadingTag::getInstance()->get (map::HeadingTag::h5) | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) |
| map::HeadingTag::getInstance()->get (map::HeadingTag::h6) | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::String) |
| Id::a                                                     | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::reference, lsp::SymbolType::String)   |
+-----------------------------------------------------------+----------------------------------------------------------------------------------------------+

## mermaidElements

```
@brief The role and the symbol type of each mermaid element that holds a name. A node declares its id. An edge refers to the ids of its two ends.
```

+----------+----------------------------------------------------------------------------------------------+
| key      | value                                                                                        |
+==========+==============================================================================================+
| Id::node | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Object) |
| Id::edge | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::reference, lsp::SymbolType::Object)   |
+----------+----------------------------------------------------------------------------------------------+

## jsonElements

```
@brief The role and the symbol type of each json element that holds a name. A key declares its text.
```

+---------+-------------------------------------------------------------------------------------------+
| key     | value                                                                                     |
+=========+===========================================================================================+
| Id::key | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Key) |
+---------+-------------------------------------------------------------------------------------------+

## cssElements

```
@brief The role and the symbol type of each name token of a css element, by the position of the token. Token 0 is the selector.
```

+-----+---------------------------------------------------------------------------------------------+
| key | value                                                                                       |
+=====+=============================================================================================+
| 0   | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Class) |
+-----+---------------------------------------------------------------------------------------------+

## xmlElements

```
@brief The role and the symbol type of each name token of an xml element, by the position of the token. Token 0 is the tag. Token 1 is the value of the id attribute.
```

+-----+------------------------------------------------------------------------------------------------+
| key | value                                                                                          |
+=====+================================================================================================+
| 0   | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Object)   |
| 1   | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Constant) |
+-----+------------------------------------------------------------------------------------------------+

## htmlElements

```
@brief The role and the symbol type of each name token of an html element, by the position of the token. Token 0 is the tag. Token 1 is the value of the id attribute.
```

+-----+------------------------------------------------------------------------------------------------+
| key | value                                                                                          |
+=====+================================================================================================+
| 0   | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Object)   |
| 1   | jam::Union<uint8_t, uint8_t>::pack (jam::LanguageRole::declaration, lsp::SymbolType::Constant) |
+-----+------------------------------------------------------------------------------------------------+
