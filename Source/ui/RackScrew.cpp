#include "RackScrew.h"
#include "GraphicsHelpers.h"

namespace RackScrew
{
void draw(juce::Graphics& g, juce::Point<float> centre, float radius, float slotAngle)
{
    const auto bounds = juce::Rectangle<float>(radius * 2.0f, radius * 2.0f).withCentre(centre);
    
    // Realistic radial gradient shadow for the screw (offset bottom-left)
    const float shadowBlur = radius * 0.65f;
    const float dx = -2.0f;
    const float dy = 3.0f;
    const float shadowRadius = radius + shadowBlur;
    const auto shadowCentre = centre.translated(dx, dy);

    juce::ColourGradient shadowGrad(
        juce::Colours::black.withAlpha(0.68f), shadowCentre.x, shadowCentre.y,
        juce::Colours::transparentBlack, shadowCentre.x + shadowRadius, shadowCentre.y,
        true);
    shadowGrad.addColour(0.40, juce::Colours::black.withAlpha(0.38f));
    g.setGradientFill(shadowGrad);
    g.fillEllipse(juce::Rectangle<float>(shadowRadius * 2.0f, shadowRadius * 2.0f).withCentre(shadowCentre));
    
    // Screw Head Base
    juce::ColourGradient baseGrad(juce::Colour(0xffa5aab2), centre.x - radius * 0.6f, centre.y - radius * 0.6f,
                                  juce::Colour(0xff181a1e), centre.x + radius, centre.y, true);
    baseGrad.addColour(0.5f, juce::Colour(0xff4a4c52));
    g.setGradientFill(baseGrad);
    g.fillEllipse(bounds);
    
    // Specular Highlight (Rim)
    juce::ColourGradient rimGrad(juce::Colours::white.withAlpha(0.6f), centre.x - radius * 0.8f, centre.y - radius * 0.8f,
                                 juce::Colours::transparentWhite, centre.x + radius * 0.3f, centre.y + radius * 0.3f, true);
    g.setGradientFill(rimGrad);
    g.drawEllipse(bounds.reduced(radius * 0.05f), radius * 0.08f);
    
    // Outer dark rim
    g.setColour(juce::Colours::black.withAlpha(0.85f));
    g.drawEllipse(bounds, radius * 0.06f);

    // 3D Slot
    g.saveState();
    g.addTransform(juce::AffineTransform::rotation(slotAngle).translated(centre.x, centre.y));
    
    // Slot hole (deep shadow)
    const float slotW = radius * 1.55f;
    const float slotH = radius * 0.34f;
    const auto slotRect = juce::Rectangle<float>(slotW, slotH).withCentre({0.0f, 0.0f});
    
    g.setColour(juce::Colour(0xff090a0c));
    g.fillRoundedRectangle(slotRect, radius * 0.1f);
    
    // Slot inner top shadow (depth)
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(slotRect.withTrimmedBottom(slotH * 0.5f).translated(0.0f, -radius * 0.04f), radius * 0.1f);
    
    // Slot inner bottom highlight (bevel catching light)
    g.setColour(juce::Colours::white.withAlpha(0.45f));
    g.drawRoundedRectangle(slotRect.translated(0.0f, radius * 0.06f), radius * 0.1f, radius * 0.06f);
    
    // Clean up outside by clipping to a slightly smaller ellipse so the slot doesn't break the screw edge
    // Wait, the slot is already 1.55f width which is < 2.0f, so it fits inside the screw.

    // Center pivot hole (some screws have a tiny dimple in the center)
    g.setColour(juce::Colour(0xff050608));
    g.fillEllipse(juce::Rectangle<float>(radius * 0.35f, radius * 0.35f).withCentre({0.0f, 0.0f}));
    
    g.restoreState();
}

} // namespace RackScrew
