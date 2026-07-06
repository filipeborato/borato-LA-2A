#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/La2aPanelComponent.h"

#if BORATO_USE_OPENGL_RENDERER
#include <juce_opengl/juce_opengl.h>
#endif

class BoratoLA2AAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BoratoLA2AAudioProcessorEditor(BoratoLA2AAudioProcessor&);
    ~BoratoLA2AAudioProcessorEditor() override;
    void resized() override;

private:
    La2aPanelComponent panel;
#if BORATO_USE_OPENGL_RENDERER
    juce::OpenGLContext openGLContext;
#endif
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoratoLA2AAudioProcessorEditor)
};
