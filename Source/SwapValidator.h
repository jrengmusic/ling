#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

struct SwapValidator : jam::Document::Validator
{
    static void addRules (Rules& rules);

    const Rules& getRules() const override;
};
