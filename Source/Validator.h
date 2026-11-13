#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

/**
 * @struct Validator
 * @brief The validator of a LING manifest: the markdown rules, and the rule for the columns of each toolchain table.
 *
 * A toolchain table needs a header row with a command column and a flag column.
 */
struct Validator : jam::MarkdownValidator
{
    /**
     * @brief Adds the markdown rules, the toolchain rule, and the daemon rule to the rule set.
     *
     * The toolchain rule fails each toolchain table whose header row has no
     * command column or no flag column. The daemon rule fails each timeout row
     * whose value is not a positive integer in canonical decimal form: no sign, no
     * leading zero, and no space.
     *
     * @param rules  The rule set that receives the entries.
     */
    static void addRules (Rules& rules);

    /**
     * @brief Returns the rule set, built once on first use.
     *
     * @return The rule set of the manifest validator.
     */
    const Rules& getRules() const override;
};
