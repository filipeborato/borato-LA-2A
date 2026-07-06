#include "JewelLightComponent.h"
#include "GraphicsHelpers.h"

void JewelLightComponent::setOn(bool shouldBeOn)
{
    if (on != shouldBeOn)
    {
        on = shouldBeOn;
        repaint();
    }
}

void JewelLightComponent::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();
    const float radius = 32.24f; // Fixed design radius (matches 124 * 0.26)

    // 1. Red glowing bloom (when ON) - sized to fit completely within 240x240 bounds (radius 90.27f)
    if (on)
    {
        juce::ColourGradient bloom(juce::Colour(0x60ff1e0a), centre.x, centre.y,
                                   juce::Colours::transparentBlack, centre.x + radius * 2.8f, centre.y, true);
        g.setGradientFill(bloom);
        g.fillEllipse(juce::Rectangle<float>(radius * 5.6f, radius * 5.6f).withCentre(centre));
    }

    // 3. Chrome Bezel Ring (3D metal reflection)
    const auto bezelArea = juce::Rectangle<float>(radius * 2.65f, radius * 2.65f).withCentre(centre);
    juce::ColourGradient bezelGrad(juce::Colour(0xffebedf2), centre.x - radius * 1.3f, centre.y - radius * 1.3f,
                                   juce::Colour(0xff15161a), centre.x + radius * 1.3f, centre.y + radius * 1.3f, false);
    bezelGrad.addColour(0.20, juce::Colour(0xffffffff));
    bezelGrad.addColour(0.42, juce::Colour(0xff7c818a));
    bezelGrad.addColour(0.68, juce::Colour(0xff292b30));
    bezelGrad.addColour(0.88, juce::Colour(0xffeceff4));
    g.setGradientFill(bezelGrad);
    g.fillEllipse(bezelArea);

    // Inner dark groove in bezel
    g.setColour(juce::Colour(0xff08090a));
    g.fillEllipse(juce::Rectangle<float>(radius * 2.18f, radius * 2.18f).withCentre(centre));
    g.setColour(juce::Colours::white.withAlpha(0.18f));
    g.drawEllipse(juce::Rectangle<float>(radius * 2.42f, radius * 2.42f).withCentre(centre), 1.0f);

    // 4. Red Glass Lens
    const auto lens = juce::Rectangle<float>(radius * 1.82f, radius * 1.82f).withCentre(centre);
    juce::ColourGradient lensGrad(
        on ? juce::Colour(0xffff553b) : juce::Colour(0xff6e1008), centre.x - radius * 0.4f, centre.y - radius * 0.4f,
        juce::Colour(0xff1f0000), centre.x + radius * 0.9f, centre.y + radius * 0.9f, true);
    lensGrad.addColour(0.55, on ? juce::Colour(0xffb80f00) : juce::Colour(0xff400502));
    g.setGradientFill(lensGrad);
    g.fillEllipse(lens);

    // 5. Crystal Facets (rendered with soft opacity for inner refraction)
    for (int i = 0; i < 12; ++i)
    {
        const float a1 = juce::MathConstants<float>::twoPi * (float) i / 12.0f;
        const float a2 = juce::MathConstants<float>::twoPi * (float) (i + 1) / 12.0f;
        juce::Path facet;
        facet.startNewSubPath(centre);
        facet.lineTo(centre.getPointOnCircumference(radius * 0.91f, a1));
        facet.lineTo(centre.getPointOnCircumference(radius * 0.91f, a2));
        facet.closeSubPath();
        
        // Soft opacity to look "behind" the glass dome
        g.setColour((i % 2 == 0 ? juce::Colour(0xffff5a40) : juce::Colour(0xff4a0300))
                        .withAlpha(on ? 0.26f : 0.08f));
        g.fillPath(facet);
        g.setColour(juce::Colour(0xff210000).withAlpha(on ? 0.45f : 0.12f));
        g.strokePath(facet, juce::PathStrokeType(0.65f));
    }

    // 6. Glass Highlights (specular light reflections)
    // Primary specular highlight (top-left)
    g.setColour(juce::Colours::white.withAlpha(on ? 0.85f : 0.45f));
    g.fillEllipse(juce::Rectangle<float>(radius * 0.28f, radius * 0.22f)
                      .withCentre(centre.translated(-radius * 0.35f, -radius * 0.42f)));
                      
    // Secondary bounce highlight (bottom-right edge)
    g.setColour(juce::Colours::white.withAlpha(on ? 0.22f : 0.08f));
    g.drawEllipse(lens.reduced(1.5f), 1.0f);
    
    g.setColour(juce::Colour(0xff160000).withAlpha(0.62f));
    g.drawEllipse(lens, 1.0f);
}
