#pragma once

#include <JuceHeader.h>

class VintageKnobComponent final : public juce::Slider
{
public:
    explicit VintageKnobComponent(bool compact = false);
    void paint(juce::Graphics&) override;

private:
    bool isCompact = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VintageKnobComponent)
};
