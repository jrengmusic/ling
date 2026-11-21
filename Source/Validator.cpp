#include "Validator.h"

void Validator::addRules (Rules& rules)
{
    jam::MarkdownValidator::addRules (rules);

    rules.add (Id::toolchain.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            const auto& manifest { static_cast<const jam::MarkdownDocument&> (document) };
            jam::Strings failures;

            for (const auto* table : manifest.getTables (Id::toolchain))
            {
                const auto* headerRow { manifest.getTableRow (*table, Id::headerRow) };

                if (manifest.getTableCell (*headerRow, Id::command) == nullptr
                    or manifest.getTableCell (*headerRow, Id::flag) == nullptr)
                    failures.add (getLocation (*table, *headerRow, Id::toolchain.toString())
                                  + Id::diagnosticSeparator
                                  + text::Diagnostics::failToolchainColumn);
            }

            return getFailures (failures);
        });

    rules.add (Id::daemon.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            static const std::array<std::pair<juce::Identifier, const char*>, 2> positiveRows { { { Id::timeout, text::Diagnostics::failDaemonTimeout },
                                                                                                    { Id::threads, text::Diagnostics::failDaemonThreads } } };
            const auto& manifest { static_cast<const jam::MarkdownDocument&> (document) };
            jam::Strings failures;

            for (const auto* table : manifest.getTables (Id::daemon))
            {
                for (const auto& [rowId, diagnostic] : positiveRows)
                {
                    if (const auto* row { manifest.getTableRow (*table, rowId) }; row != nullptr)
                    {
                        const auto value { manifest.getTableValue (*row, Id::value) };

                        if (juce::String (value.getIntValue()).compare (value) != 0 or value.getIntValue() <= 0)
                            failures.add (getLocation (*table, *row, Id::value.toString())
                                          + Id::diagnosticSeparator
                                          + diagnostic);
                    }
                }
            }

            return getFailures (failures);
        });
}

const Validator::Rules& Validator::getRules() const
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
