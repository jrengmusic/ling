#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

struct SymbolDocument : jam::Document
{
    SymbolDocument() = default;

    static std::unique_ptr<SymbolDocument> read (std::string_view stored);

    template <typename Function>
    static void applyToRecords (const jam::Document::Element& file, const Function& function)
    {
        for (const auto* child : file)
            if (child->id == Id::symbol)
                function (*child);
    }

    template <typename Function>
    static void applyToCompletions (const jam::Document::Element& file, const Function& function)
    {
        for (const auto* container : file)
            if (container->id == Id::completions)
                for (const auto* completion : *container)
                    if (completion->id == Id::completion)
                        function (*completion);
    }

    static constexpr juce::int64 noSymbol { 0 };
    static constexpr int noParent { -1 };

protected:
    int getToken (Cursor& cursor, int segmentType) override;
    void build() override;
    const Vocabulary& getVocabulary() const override;

private:
    static constexpr const char* emptySource { "" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SymbolDocument)
};
