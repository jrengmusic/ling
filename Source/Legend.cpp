#include "Legend.h"
#include "generated/Generated.h"

static int getNameIndex (const jam::Strings& names, juce::StringRef name)
{
    const auto index { names.indexOf (name, false, 0) };

    jassert (index >= 0);
    return index;
}

const jam::Strings& Legend::getTypes()
{
    static const jam::Strings types (buildTypes());
    return types;
}

const jam::Strings& Legend::getModifiers()
{
    static const jam::Strings modifiers (buildModifiers());
    return modifiers;
}

int Legend::getTypeIndex (juce::StringRef name)
{
    return getNameIndex (getTypes(), name);
}

int Legend::getModifierBit (juce::StringRef name)
{
    return 1 << getNameIndex (getModifiers(), name);
}

jam::Strings Legend::buildTypes()
{
    jam::Strings types;

    addNames (types, lsp::SemanticTokenType {});
    addNames (types, map::MarkdownTokenType {});
    addNames (types, map::DocumentTokenType {});
    addNames (types, map::MermaidTokenType {});
    addNames (types, map::JsonTokenType {});
    addNames (types, map::CssTokenType {});
    addNames (types, map::XmlTokenType {});
    addNames (types, map::HtmlTokenType {});

    return types;
}

jam::Strings Legend::buildModifiers()
{
    jam::Strings modifiers;

    addNames (modifiers, lsp::SemanticTokenModifier {});

    return modifiers;
}

void Legend::addNames (jam::Strings& names, const jam::Bimap<int>& tokenNames)
{
    for (const auto& [tokenKey, tokenName] : tokenNames.get())
        if (not names.contains (tokenName, false))
            names.add (tokenName);
}
