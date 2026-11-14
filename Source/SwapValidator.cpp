#include "SwapValidator.h"

void SwapValidator::addRules (Rules& rules)
{
    rules.add (Id::version.toString(),
        [] (const juce::String&, const jam::Document& document) -> juce::Result
        {
            jam::Strings failures;
            const auto* version { document.getRoot()->find<juce::String> (Id::version) };

            if (version == nullptr or version->compare (ProjectInfo::versionString) != 0)
                failures.add (juce::String { text::Diagnostics::failSwapVersion });

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
