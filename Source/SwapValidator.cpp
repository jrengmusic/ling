#include "SwapValidator.h"

void SwapValidator::addRules (Rules& rules)
{
    static const std::array<juce::Identifier, 3> rootFields { Id::hash, Id::language, Id::family };

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
