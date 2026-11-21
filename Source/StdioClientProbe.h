#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

#if LING_USING_SCHEMA
/**
 * @struct StdioClientProbe
 * @brief Exercises jam::StdioClient against stdioClient manifest rows when LING_USING_SCHEMA is enabled.
 */
struct StdioClientProbe
{
    /**
     * @brief Launches each Id::stdioClient row in @p manifest, feeds optional input, and prints byte count and exit code.
     *
     * @param manifest  Parsed manifest document containing Id::stdioClient table rows.
     * @param folder    Working directory passed to each launched subprocess.
     * @return juce::Result::ok() after all probe rows finish.
     */
    static juce::Result run (const jam::MarkdownDocument& manifest, const juce::File& folder);
};
#endif
