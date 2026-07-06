#pragma once

#include <JuceHeader.h>

class JewelLightComponent final : public juce::Component
{
public:
    JewelLightComponent() { setOpaque(false); }
    void setOn(bool);
    void paint(juce::Graphics&) override;

private:
    bool on = true;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JewelLightComponent)
};
