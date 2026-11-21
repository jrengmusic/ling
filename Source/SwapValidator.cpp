#include "SwapValidator.h"
#include "SymbolDocument.h"

template <typename Item>
static bool hasFields (const Item& item, const jam::Array<juce::Identifier>& fields)
{
    return std::all_of (fields.begin(), fields.end(), [&item] (const juce::Identifier& field) { return item.contains (field); });
}

void SwapValidator::addRules (Rules& rules)
{
    static const std::array<juce::Identifier, 3> rootFields { Id::hash, Id::language, Id::family };
    static const jam::Array<juce::Identifier> fileFields { Id::path, Id::inlayHints, Id::diagnostics, Id::outline, Id::fixIts, Id::hovers };
    static const jam::Array<juce::Identifier> recordFields { Id::line, Id::column, Id::nameLength, Id::role, Id::type, Id::symbol, Id::hover, Id::typeSymbols };
    static const jam::Array<juce::Identifier> completionFields { Id::scope, Id::symbol, Id::label, Id::snippet, Id::type };
    static const std::array<std::pair<juce::Identifier, jam::Array<juce::Identifier>>, 4> tokenFields { {
        { Id::inlayHints, jam::Array<juce::Identifier> { Id::line, Id::column, Id::type, Id::label, Id::paddingLeft, Id::paddingRight } },
        { Id::diagnostics, jam::Array<juce::Identifier> { Id::line, Id::column, Id::endLine, Id::endColumn, Id::severity, Id::message } },
        { Id::outline, jam::Array<juce::Identifier> { Id::line, Id::column, Id::endLine, Id::endColumn, Id::nameLength, Id::qualifierLength, Id::type, Id::parent } },
        { Id::fixIts, jam::Array<juce::Identifier> { Id::line, Id::column, Id::endLine, Id::endColumn, Id::text } } } };

    rules.add (Id::version.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            jam::Strings failures;

            if (not document.getRoot()->contains (Id::version) or document.getRoot()->get<juce::String> (Id::version)->compare (ProjectInfo::versionString) != 0)
                failures.add (juce::String { text::Diagnostics::failSwapVersion });

            return getFailures (failures);
        });

    for (const auto& field : rootFields)
        rules.add (field.toString(),
            [&field] (const juce::String&, const jam::Document& document) -> juce::Result
            {
                jam::Strings failures;

                if (not document.getRoot()->contains (field))
                    failures.add (juce::String { text::Diagnostics::failSwapRecord });

                return getFailures (failures);
            });

    rules.add (Id::file.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            jam::Strings failures;

            for (const auto* file : *document.getRoot())
            {
                if (file->id == Id::file and not hasFields (*file, fileFields))
                    failures.add (juce::String { text::Diagnostics::failSwapRecord });
            }

            return getFailures (failures);
        });

    for (const auto& entry : tokenFields)
    {
        const auto& [tokensId, fields] { entry };

        rules.add (tokensId.toString(),
            [&entry] (const juce::String&, const jam::Document& document) -> juce::Result
            {
                const auto& [tokensId, fields] { entry };
                jam::Strings failures;

                for (const auto* file : *document.getRoot())
                {
                    if (file->contains (tokensId))
                    {
                        for (const auto& token : *file->get<jam::Document::Tokens> (tokensId))
                        {
                            if (not hasFields (token, fields))
                                failures.add (juce::String { text::Diagnostics::failSwapRecord });
                        }
                    }
                }

                return getFailures (failures);
            });
    }

    rules.add (Id::symbol.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            jam::Strings failures;

            for (const auto* file : *document.getRoot())
            {
                SymbolDocument::applyToRecords (*file, [&failures] (const jam::Document::Element& record)
                {
                    if (not hasFields (record, recordFields))
                        failures.add (juce::String { text::Diagnostics::failSwapRecord });
                });
            }

            return getFailures (failures);
        });

    rules.add (Id::completions.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            jam::Strings failures;

            for (const auto* file : *document.getRoot())
            {
                if (file->id == Id::file)
                {
                    int containerCount { 0 };

                    for (const auto* container : *file)
                    {
                        if (container->id == Id::completions)
                            ++containerCount;
                    }

                    if (containerCount != 1)
                        failures.add (juce::String { text::Diagnostics::failSwapRecord });

                    SymbolDocument::applyToCompletions (*file, [&failures] (const jam::Document::Element& completion)
                    {
                        if (not hasFields (completion, completionFields))
                            failures.add (juce::String { text::Diagnostics::failSwapRecord });
                    });
                }
            }

            return getFailures (failures);
        });
}

const SwapValidator::Rules& SwapValidator::getRules() const
{
    static const auto rules (
        []
        {
            Rules rules;
            addRules (rules);
            return rules;
        }()
    );

    return rules;
}
