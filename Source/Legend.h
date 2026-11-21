#pragma once
#include <JuceHeader.h>

/**
 * @class Legend
 * @brief Provides the unified LSP semantic token type and modifier tables shared across all languages.
 */
class Legend
{
public:
    /**
     * @brief Returns the deduplicated semantic token type names advertised in server capabilities.
     *
     * @return Const reference to the ordered semantic token type list.
     */
    static const jam::Strings& getTypes();

    /**
     * @brief Returns the semantic token modifier names advertised in server capabilities.
     *
     * @return Const reference to the ordered semantic token modifier list.
     */
    static const jam::Strings& getModifiers();

    /**
     * @brief Returns the zero-based legend index of @p name, asserting that it exists.
     *
     * @param name  Semantic token type name registered in getTypes().
     * @return Zero-based index in getTypes().
     */
    static int getTypeIndex (juce::StringRef name);

    /**
     * @brief Returns the bitmask (`1 << index`) for modifier @p name, asserting that it exists.
     *
     * @param name  Semantic token modifier name registered in getModifiers().
     * @return Single-bit integer mask for @p name.
     */
    static int getModifierBit (juce::StringRef name);

private:
    /**
     * @brief Builds the merged list of LSP standard types and built-in grammar token types.
     *
     * @return Ordered list of unique semantic token type names.
     */
    static jam::Strings buildTypes();

    /**
     * @brief Builds the list of LSP standard semantic token modifier names.
     *
     * @return Ordered list of unique semantic token modifier names.
     */
    static jam::Strings buildModifiers();

    /**
     * @brief Appends each name from @p tokenNames not already present in @p names.
     *
     * @param names       Target string list receiving deduplicated entries.
     * @param tokenNames  Bimap providing token type or modifier names.
     */
    static void addNames (jam::Strings& names, const jam::Bimap<int>& tokenNames);
};
