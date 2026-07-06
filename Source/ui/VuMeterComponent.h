#pragma once

#include <JuceHeader.h>

class VuMeterComponent final : public juce::Component, private juce::Timer
{
public:
    VuMeterComponent();
    ~VuMeterComponent() override;
    void setDb(float db);
    void setReflectionOffset(float x, float y);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    static float dbToAngle(float db);
    void rebuildStaticCache();
    void timerCallback() override;

    juce::Image staticCache;
    float targetDb = -20.0f;
    float displayedDb = -20.0f;
    float reflectionX = 0.0f;
    float reflectionY = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VuMeterComponent)
};
