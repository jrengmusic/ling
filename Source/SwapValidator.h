#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

/**
 * @struct SwapValidator
 * @brief Validates deserialized swap jam::Document trees before Index::readSwapFile() accepts them.
 */
struct SwapValidator : jam::Document::Validator
{
    /**
     * @brief Registers root-element rules requiring matching Id::version and presence of Id::hash, Id::language, and Id::family.
     *
     * @param rules  The rule set that receives the swap validation entries.
     */
    static void addRules (Rules& rules);

    /**
     * @brief Returns the swap validation rule set, built once on first use.
     *
     * @return The rule set of the swap validator.
     */
    const Rules& getRules() const override;
};
