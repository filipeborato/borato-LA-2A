#pragma once

#include <JuceHeader.h>

class ToggleSwitchComponent final : public juce::Button
{
public:
    ToggleSwitchComponent();
    void paint(juce::Graphics&) override;
    void paintButton(juce::Graphics&, bool, bool) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSwitchComponent)
};
