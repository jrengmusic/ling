#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

#if LING_USING_SCHEMA
struct StdioClientProbe
{
    static juce::Result run (const jam::MarkdownDocument& manifest, const juce::File& folder);

private:
    static constexpr int bufferSize { 1 << 20 };
    static constexpr double maximumPeriodMilliseconds { 1.0 };
    static constexpr std::string_view lineSeparator { ": " };
    static constexpr std::string_view bytesLabel { "bytes=" };
    static constexpr std::string_view exitLabel { " exit=" };
    static constexpr std::string_view standardOutput { "out" };
    static constexpr std::string_view joinedOutput { "out+err" };
};
#endif
