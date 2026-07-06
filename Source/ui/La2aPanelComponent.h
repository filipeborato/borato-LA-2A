#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "VuMeterComponent.h"
#include "VintageKnobComponent.h"
#include "ToggleSwitchComponent.h"
#include "JewelLightComponent.h"
#include "R37TrimComponent.h"

class La2aPanelComponent final : public juce::Component, private juce::Timer
{
public:
    explicit La2aPanelComponent(BoratoLA2AAudioProcessor&);
    ~La2aPanelComponent() override;
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void setParallaxEnabled(bool enabled) noexcept { parallaxEnabled = enabled; }

private:
    static constexpr float designWidth = 1440.0f;
    static constexpr float designHeight = 1080.0f;
    juce::Rectangle<float> getDesignArea() const;
    juce::Rectangle<int> mapRect(juce::Rectangle<float>) const;
    juce::AffineTransform designTransform() const;
    void rebuildBackgroundCache();
    void drawStaticPanel(juce::Graphics&);
    void timerCallback() override;

    BoratoLA2AAudioProcessor& processor;
    VuMeterComponent meter;
    VintageKnobComponent gainKnob;
    VintageKnobComponent peakKnob;
    VintageKnobComponent meterSelector { true };
    ToggleSwitchComponent modeSwitch;
    ToggleSwitchComponent powerSwitch;
    JewelLightComponent jewelLight;
    R37TrimComponent r37Knob;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> meterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> r37Attachment;

    juce::Image backgroundCache;
    juce::Image panelNoise;
    float targetTiltX = 0.0f;
    float targetTiltY = 0.0f;
    float tiltX = 0.0f;
    float tiltY = 0.0f;
    bool parallaxEnabled = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(La2aPanelComponent)
};
