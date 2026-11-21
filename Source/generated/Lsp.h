/*******************************************************************************
                        Codegen Annotated Source of Truth
————————————————————————————————————————————————————————————————————————————————

            ░░████████████░░████████████░░████████████░░████████████
            ░░████  ░░████░░████  ░░████░░████  ░░████    ░░████
            ░░████        ░░████  ░░████░░████            ░░████
            ░░████        ░░████████████░░████████████    ░░████
            ░░████        ░░████  ░░████        ░░████    ░░████
            ░░████  ░░████░░████  ░░████░░████  ░░████    ░░████
            ░░████████████░░████  ░░████░░████████████    ░░████

————————————————————————————————————————————————————————————————————————————————
                         FOR YOUR EYES ONLY, DO NOT EDIT
********************************************************************************/

/**
 * @file Lsp.h
 * @brief The LSP 3.17 method names, types, and codes that ling serves.
 */

#pragma once

namespace lsp
{
/*_____________________________________________________________________________*/

/** @brief The LSP 3.17 method names that ling serves. */
struct Method
{
    static constexpr std::string_view initialize           { "initialize"                        };
    static constexpr std::string_view initialized          { "initialized"                       };
    static constexpr std::string_view shutdown             { "shutdown"                          };
    static constexpr std::string_view exit                 { "exit"                              };
    static constexpr std::string_view cancelRequest        { "$/cancelRequest"                   };
    static constexpr std::string_view didOpen              { "textDocument/didOpen"              };
    static constexpr std::string_view didChange            { "textDocument/didChange"            };
    static constexpr std::string_view didClose             { "textDocument/didClose"             };
    static constexpr std::string_view publishDiagnostics   { "textDocument/publishDiagnostics"   };
    static constexpr std::string_view showMessage          { "window/showMessage"                };
    static constexpr std::string_view progress             { "$/progress"                        };
    static constexpr std::string_view definition           { "textDocument/definition"           };
    static constexpr std::string_view references           { "textDocument/references"           };
    static constexpr std::string_view hover                { "textDocument/hover"                };
    static constexpr std::string_view documentSymbol       { "textDocument/documentSymbol"       };
    static constexpr std::string_view workspaceSymbol      { "workspace/symbol"                  };
    static constexpr std::string_view completion           { "textDocument/completion"           };
    static constexpr std::string_view implementation       { "textDocument/implementation"       };
    static constexpr std::string_view prepareCallHierarchy { "textDocument/prepareCallHierarchy" };
    static constexpr std::string_view incomingCalls        { "callHierarchy/incomingCalls"       };
    static constexpr std::string_view outgoingCalls        { "callHierarchy/outgoingCalls"       };
    static constexpr std::string_view inlayHint            { "textDocument/inlayHint"            };
    static constexpr std::string_view declaration          { "textDocument/declaration"          };
    static constexpr std::string_view typeDefinition       { "textDocument/typeDefinition"       };
    static constexpr std::string_view documentHighlight    { "textDocument/documentHighlight"    };
    static constexpr std::string_view rename               { "textDocument/rename"               };
    static constexpr std::string_view codeAction           { "textDocument/codeAction"           };
    static constexpr std::string_view switchSourceHeader   { "textDocument/switchSourceHeader"   };
};
/** @brief The LSP 3.17 member names of the messages that ling writes. */
struct Member
{
    static constexpr std::string_view uri                       { "uri"                       };
    static constexpr std::string_view range                     { "range"                     };
    static constexpr std::string_view start                     { "start"                     };
    static constexpr std::string_view end                       { "end"                       };
    static constexpr std::string_view line                      { "line"                      };
    static constexpr std::string_view character                 { "character"                 };
    static constexpr std::string_view selectionRange            { "selectionRange"            };
    static constexpr std::string_view name                      { "name"                      };
    static constexpr std::string_view type                      { "kind"                      };
    static constexpr std::string_view children                  { "children"                  };
    static constexpr std::string_view contents                  { "contents"                  };
    static constexpr std::string_view value                     { "value"                     };
    static constexpr std::string_view label                     { "label"                     };
    static constexpr std::string_view position                  { "position"                  };
    static constexpr std::string_view paddingLeft               { "paddingLeft"               };
    static constexpr std::string_view paddingRight              { "paddingRight"              };
    static constexpr std::string_view severity                  { "severity"                  };
    static constexpr std::string_view message                   { "message"                   };
    static constexpr std::string_view documentation             { "documentation"             };
    static constexpr std::string_view insertText                { "insertText"                };
    static constexpr std::string_view insertTextFormat          { "insertTextFormat"          };
    static constexpr std::string_view from                      { "from"                      };
    static constexpr std::string_view to                        { "to"                        };
    static constexpr std::string_view fromRanges                { "fromRanges"                };
    static constexpr std::string_view location                  { "location"                  };
    static constexpr std::string_view result                    { "result"                    };
    static constexpr std::string_view method                    { "method"                    };
    static constexpr std::string_view params                    { "params"                    };
    static constexpr std::string_view messageType               { "type"                      };
    static constexpr std::string_view jsonrpc                   { "jsonrpc"                   };
    static constexpr std::string_view id                        { "id"                        };
    static constexpr std::string_view error                     { "error"                     };
    static constexpr std::string_view code                      { "code"                      };
    static constexpr std::string_view textDocument              { "textDocument"              };
    static constexpr std::string_view text                      { "text"                      };
    static constexpr std::string_view contentChanges            { "contentChanges"            };
    static constexpr std::string_view query                     { "query"                     };
    static constexpr std::string_view item                      { "item"                      };
    static constexpr std::string_view capabilities              { "capabilities"              };
    static constexpr std::string_view general                   { "general"                   };
    static constexpr std::string_view positionEncodings         { "positionEncodings"         };
    static constexpr std::string_view positionEncoding          { "positionEncoding"          };
    static constexpr std::string_view textDocumentSync          { "textDocumentSync"          };
    static constexpr std::string_view hoverProvider             { "hoverProvider"             };
    static constexpr std::string_view definitionProvider        { "definitionProvider"        };
    static constexpr std::string_view referencesProvider        { "referencesProvider"        };
    static constexpr std::string_view documentSymbolProvider    { "documentSymbolProvider"    };
    static constexpr std::string_view workspaceSymbolProvider   { "workspaceSymbolProvider"   };
    static constexpr std::string_view completionProvider        { "completionProvider"        };
    static constexpr std::string_view implementationProvider    { "implementationProvider"    };
    static constexpr std::string_view callHierarchyProvider     { "callHierarchyProvider"     };
    static constexpr std::string_view inlayHintProvider         { "inlayHintProvider"         };
    static constexpr std::string_view semanticTokensProvider    { "semanticTokensProvider"    };
    static constexpr std::string_view selectionRangeProvider    { "selectionRangeProvider"    };
    static constexpr std::string_view serverInfo                { "serverInfo"                };
    static constexpr std::string_view version                   { "version"                   };
    static constexpr std::string_view diagnostics               { "diagnostics"               };
    static constexpr std::string_view legend                    { "legend"                    };
    static constexpr std::string_view tokenTypes                { "tokenTypes"                };
    static constexpr std::string_view tokenModifiers            { "tokenModifiers"            };
    static constexpr std::string_view full                      { "full"                      };
    static constexpr std::string_view data                      { "data"                      };
    static constexpr std::string_view positions                 { "positions"                 };
    static constexpr std::string_view parent                    { "parent"                    };
    static constexpr std::string_view declarationProvider       { "declarationProvider"       };
    static constexpr std::string_view documentHighlightProvider { "documentHighlightProvider" };
    static constexpr std::string_view renameProvider            { "renameProvider"            };
    static constexpr std::string_view changes                   { "changes"                   };
    static constexpr std::string_view newText                   { "newText"                   };
    static constexpr std::string_view title                     { "title"                     };
    static constexpr std::string_view edit                      { "edit"                      };
    static constexpr std::string_view typeDefinitionProvider    { "typeDefinitionProvider"    };
    static constexpr std::string_view codeActionProvider        { "codeActionProvider"        };
    static constexpr std::string_view token                     { "token"                     };
    static constexpr std::string_view percentage                { "percentage"                };
};
/** @brief The LSP 3.17 MarkupKind values. */
struct MarkupType
{
    static constexpr std::string_view PlainText { "plaintext" };
    static constexpr std::string_view Markdown  { "markdown"  };
};
/** @brief The LSP 3.17 WorkDoneProgressKind values. */
struct WorkDoneProgressKind
{
    static constexpr std::string_view Begin  { "begin"  };
    static constexpr std::string_view Report { "report" };
    static constexpr std::string_view End    { "end"    };
};
/** @brief The LSP 3.17 base protocol version. */
struct Message
{
    static constexpr std::string_view jsonrpc { "2.0" };
};
/** @brief The LSP 3.17 MessageType values. */
struct MessageType
{
    static constexpr int Error   { 1 };
    static constexpr int Warning { 2 };
    static constexpr int Info    { 3 };
    static constexpr int Log     { 4 };
};
/** @brief The LSP 3.17 InsertTextFormat values. */
struct InsertTextFormat
{
    static constexpr int PlainText { 1 };
    static constexpr int Snippet   { 2 };
};
/** @brief The LSP 3.17 SymbolKind values. */
struct SymbolType
{
    static constexpr int File          { 1  };
    static constexpr int Module        { 2  };
    static constexpr int Namespace     { 3  };
    static constexpr int Package       { 4  };
    static constexpr int Class         { 5  };
    static constexpr int Method        { 6  };
    static constexpr int Property      { 7  };
    static constexpr int Field         { 8  };
    static constexpr int Constructor   { 9  };
    static constexpr int Enum          { 10 };
    static constexpr int Interface     { 11 };
    static constexpr int Function      { 12 };
    static constexpr int Variable      { 13 };
    static constexpr int Constant      { 14 };
    static constexpr int String        { 15 };
    static constexpr int Number        { 16 };
    static constexpr int Boolean       { 17 };
    static constexpr int Array         { 18 };
    static constexpr int Object        { 19 };
    static constexpr int Key           { 20 };
    static constexpr int Null          { 21 };
    static constexpr int EnumMember    { 22 };
    static constexpr int Struct        { 23 };
    static constexpr int Event         { 24 };
    static constexpr int Operator      { 25 };
    static constexpr int TypeParameter { 26 };
};
/** @brief The LSP 3.17 CompletionItemKind values. */
struct CompletionItemType
{
    static constexpr int Text          { 1  };
    static constexpr int Method        { 2  };
    static constexpr int Function      { 3  };
    static constexpr int Constructor   { 4  };
    static constexpr int Field         { 5  };
    static constexpr int Variable      { 6  };
    static constexpr int Class         { 7  };
    static constexpr int Interface     { 8  };
    static constexpr int Module        { 9  };
    static constexpr int Property      { 10 };
    static constexpr int Unit          { 11 };
    static constexpr int Value         { 12 };
    static constexpr int Enum          { 13 };
    static constexpr int Keyword       { 14 };
    static constexpr int Snippet       { 15 };
    static constexpr int Color         { 16 };
    static constexpr int File          { 17 };
    static constexpr int Reference     { 18 };
    static constexpr int Folder        { 19 };
    static constexpr int EnumMember    { 20 };
    static constexpr int Constant      { 21 };
    static constexpr int Struct        { 22 };
    static constexpr int Event         { 23 };
    static constexpr int Operator      { 24 };
    static constexpr int TypeParameter { 25 };
};
/** @brief The LSP 3.17 InlayHintKind values. */
struct InlayHintType
{
    static constexpr int Type      { 1 };
    static constexpr int Parameter { 2 };
};
/** @brief The LSP 3.17 DocumentHighlightKind values. */
struct DocumentHighlightType
{
    static constexpr int Text  { 1 };
    static constexpr int Read  { 2 };
    static constexpr int Write { 3 };
};
/** @brief The LSP 3.17 CodeActionKind values. */
struct CodeActionKind
{
    static constexpr std::string_view QuickFix { "quickfix" };
};
/** @brief The LSP 3.17 DiagnosticSeverity values. */
struct DiagnosticSeverity
{
    static constexpr int Error       { 1 };
    static constexpr int Warning     { 2 };
    static constexpr int Information { 3 };
    static constexpr int Hint        { 4 };
};
/** @brief The LSP 3.17 TextDocumentSyncKind values. */
struct TextDocumentSyncType
{
    static constexpr int None        { 0 };
    static constexpr int Full        { 1 };
    static constexpr int Incremental { 2 };
};
/** @brief The LSP 3.17 PositionEncodingKind values. */
struct PositionEncodingType
{
    static constexpr std::string_view UTF8  { "utf-8"  };
    static constexpr std::string_view UTF16 { "utf-16" };
    static constexpr std::string_view UTF32 { "utf-32" };
};
/** @brief The LSP 3.17 ErrorCodes and LSPErrorCodes values. */
struct ErrorCodes
{
    static constexpr int ParseError           { -32700 };
    static constexpr int InvalidRequest       { -32600 };
    static constexpr int MethodNotFound       { -32601 };
    static constexpr int InvalidParams        { -32602 };
    static constexpr int InternalError        { -32603 };
    static constexpr int ServerNotInitialized { -32002 };
    static constexpr int UnknownErrorCode     { -32001 };
    static constexpr int RequestFailed        { -32803 };
    static constexpr int ServerCancelled      { -32802 };
    static constexpr int ContentModified      { -32801 };
    static constexpr int RequestCancelled     { -32800 };
};
/** @brief The LSP 3.17 SemanticTokenTypes predefined values, in legend order. */
struct SemanticTokenType : public jam::Bimap<int>
{
    SemanticTokenType() : jam::Bimap<int> { {
            { namespaceToken, juce::String::fromUTF8 ("namespace") },
            { type,           juce::String::fromUTF8 ("type") },
            { classToken,     juce::String::fromUTF8 ("class") },
            { enumToken,      juce::String::fromUTF8 ("enum") },
            { interfaceToken, juce::String::fromUTF8 ("interface") },
            { structToken,    juce::String::fromUTF8 ("struct") },
            { typeParameter,  juce::String::fromUTF8 ("typeParameter") },
            { parameter,      juce::String::fromUTF8 ("parameter") },
            { variable,       juce::String::fromUTF8 ("variable") },
            { property,       juce::String::fromUTF8 ("property") },
            { enumMember,     juce::String::fromUTF8 ("enumMember") },
            { event,          juce::String::fromUTF8 ("event") },
            { function,       juce::String::fromUTF8 ("function") },
            { method,         juce::String::fromUTF8 ("method") },
            { macro,          juce::String::fromUTF8 ("macro") },
            { keyword,        juce::String::fromUTF8 ("keyword") },
            { modifier,       juce::String::fromUTF8 ("modifier") },
            { comment,        juce::String::fromUTF8 ("comment") },
            { string,         juce::String::fromUTF8 ("string") },
            { number,         juce::String::fromUTF8 ("number") },
            { regexp,         juce::String::fromUTF8 ("regexp") },
            { operatorToken,  juce::String::fromUTF8 ("operator") },
            { decorator,      juce::String::fromUTF8 ("decorator") },
    } } {}

    enum value : int
    {
        namespaceToken = 0,
        type           = 1,
        classToken     = 2,
        enumToken      = 3,
        interfaceToken = 4,
        structToken    = 5,
        typeParameter  = 6,
        parameter      = 7,
        variable       = 8,
        property       = 9,
        enumMember     = 10,
        event          = 11,
        function       = 12,
        method         = 13,
        macro          = 14,
        keyword        = 15,
        modifier       = 16,
        comment        = 17,
        string         = 18,
        number         = 19,
        regexp         = 20,
        operatorToken  = 21,
        decorator      = 22,
    };

    static SemanticTokenType* getInstance() noexcept
    {
        return jam::SharedInstance<SemanticTokenType>::getInstance();
    }
};
/** @brief The LSP 3.17 SemanticTokenModifiers predefined values, in legend order. */
struct SemanticTokenModifier : public jam::Bimap<int>
{
    SemanticTokenModifier() : jam::Bimap<int> { {
            { declaration,    juce::String::fromUTF8 ("declaration") },
            { definition,     juce::String::fromUTF8 ("definition") },
            { readonly,       juce::String::fromUTF8 ("readonly") },
            { staticToken,    juce::String::fromUTF8 ("static") },
            { deprecated,     juce::String::fromUTF8 ("deprecated") },
            { abstract,       juce::String::fromUTF8 ("abstract") },
            { async,          juce::String::fromUTF8 ("async") },
            { modification,   juce::String::fromUTF8 ("modification") },
            { documentation,  juce::String::fromUTF8 ("documentation") },
            { defaultLibrary, juce::String::fromUTF8 ("defaultLibrary") },
    } } {}

    enum value : int
    {
        declaration    = 0,
        definition     = 1,
        readonly       = 2,
        staticToken    = 3,
        deprecated     = 4,
        abstract       = 5,
        async          = 6,
        modification   = 7,
        documentation  = 8,
        defaultLibrary = 9,
    };

    static SemanticTokenModifier* getInstance() noexcept
    {
        return jam::SharedInstance<SemanticTokenModifier>::getInstance();
    }
};

/**______________________________END OF NAMESPACE______________________________*/
}// namespace lsp
