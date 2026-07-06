#pragma once

#include <JuceHeader.h>

namespace GraphicsHelpers
{
juce::Image createNoiseTexture(int width, int height, uint32_t seed = 0x4c413241u);
void drawDropShadowApprox(juce::Graphics&, juce::Rectangle<float>, float radius, juce::Colour, float offsetX, float offsetY);
void drawRadialGradientEllipse(juce::Graphics&, juce::Rectangle<float>, juce::Colour inner, juce::Colour outer,
                               juce::Point<float> focus = { 0.35f, 0.28f });
void drawLinearGradientRect(juce::Graphics&, juce::Rectangle<float>, juce::Colour top, juce::Colour bottom);
void drawPanelTexture(juce::Graphics&, const juce::Image&, juce::Rectangle<float>, float opacity);
void drawScratches(juce::Graphics&, juce::Rectangle<float>, uint32_t seed, int count, float opacity);
void drawGlassReflection(juce::Graphics&, juce::Rectangle<float>, float parallaxX, float parallaxY);
void drawEngravedText(juce::Graphics&, const juce::String&, juce::Rectangle<float>, float fontHeight,
                      juce::Justification = juce::Justification::centred, float tracking = 0.08f);
}
