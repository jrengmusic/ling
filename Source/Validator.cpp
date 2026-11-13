#include "Validator.h"

void Validator::addRules (Rules& rules)
{
    jam::MarkdownValidator::addRules (rules);

    rules.add (Id::toolchain.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            const auto& config { static_cast<const jam::ConfigDocument&> (document) };
            jam::Strings failures;

            for (const auto* table : config.getTables (Id::toolchain))
            {
                const auto* headerRow { config.getTableRow (*table, Id::headerRow) };

                if (config.getTableCell (*headerRow, Id::command) == nullptr
                    or config.getTableCell (*headerRow, Id::flag) == nullptr)
                    failures.add (getLocation (*table, *headerRow, Id::toolchain.toString())
                                  + Id::diagnosticSeparator
                                  + text::Diagnostics::failToolchainColumn);
            }

            return getFailures (failures);
        });

    rules.add (Id::daemon.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            const auto& config { static_cast<const jam::ConfigDocument&> (document) };
            jam::Strings failures;

            for (const auto* table : config.getTables (Id::daemon))
            {
                if (const auto* row { config.getTableRow (*table, Id::timeout) }; row != nullptr)
                {
                    const auto value { config.getTableValue (*row, Id::value) };

                    if (juce::String (value.getIntValue()).compare (value) != 0 or value.getIntValue() <= 0)
                        failures.add (getLocation (*table, *row, Id::value.toString())
                                      + Id::diagnosticSeparator
                                      + text::Diagnostics::failDaemonTimeout);
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
