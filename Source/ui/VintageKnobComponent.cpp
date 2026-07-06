#include "VintageKnobComponent.h"
#include "GraphicsHelpers.h"

VintageKnobComponent::VintageKnobComponent(bool compact) : isCompact(compact)
{
    setOpaque(false);
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    if (isCompact)
        setRotaryParameters(juce::degreesToRadians(322.0f), juce::degreesToRadians(398.0f), true);
    else
        setRotaryParameters(juce::degreesToRadians(225.0f), juce::degreesToRadians(495.0f), true);
    setMouseDragSensitivity(220);
}

void VintageKnobComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(isCompact ? 3.0f : 6.0f);
    const auto centre = bounds.getCentre();
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float outerRadius = radius * (isCompact ? 0.76f : 0.55f);
    const int toothCount = isCompact ? 50 : (radius > 85.0f ? 84 : 64);

    // Realistic radial gradient drop shadow
    const float shadowBlur = isCompact ? 7.0f : 42.0f;
    const float dx = isCompact ? -2.0f : -10.0f;
    const float dy = isCompact ? 3.0f : 15.0f;
    const float shadowRadius = outerRadius + shadowBlur;
    const auto shadowCentre = centre.translated(dx, dy);

    juce::ColourGradient shadowGrad(
        juce::Colours::black.withAlpha(isCompact ? 0.55f : 0.76f), shadowCentre.x, shadowCentre.y,
        juce::Colours::transparentBlack, shadowCentre.x + shadowRadius, shadowCentre.y,
        true);
    shadowGrad.addColour(0.30, juce::Colours::black.withAlpha(isCompact ? 0.35f : 0.48f));
    shadowGrad.addColour(0.60, juce::Colours::black.withAlpha(isCompact ? 0.16f : 0.18f));
    shadowGrad.addColour(0.85, juce::Colours::black.withAlpha(0.03f));

    g.setGradientFill(shadowGrad);
    g.fillEllipse(juce::Rectangle<float>(shadowRadius * 2.0f, shadowRadius * 2.0f).withCentre(shadowCentre));

    // Tight contact shadow
    const float contactRadius = outerRadius * 1.02f;
    const auto contactCentre = centre.translated(dx * 0.25f, dy * 0.25f);
    juce::ColourGradient contactGrad(
        juce::Colours::black.withAlpha(0.65f), contactCentre.x, contactCentre.y,
        juce::Colours::transparentBlack, contactCentre.x + contactRadius, contactCentre.y,
        true);
    contactGrad.addColour(0.65, juce::Colours::black.withAlpha(0.38f));
    g.setGradientFill(contactGrad);
    g.fillEllipse(juce::Rectangle<float>(contactRadius * 2.0f, contactRadius * 2.0f).withCentre(contactCentre));

    // 1. Skirt Base (Dark bakelite plastic dome)
    juce::ColourGradient skirtGrad(juce::Colour(0xff2b2d31), centre.x - outerRadius * 0.4f, centre.y - outerRadius * 0.4f,
                                   juce::Colour(0xff030304), centre.x + outerRadius, centre.y, true);
    skirtGrad.addColour(0.4f, juce::Colour(0xff121315));
    g.setGradientFill(skirtGrad);
    g.fillEllipse(juce::Rectangle<float>(outerRadius * 2.0f, outerRadius * 2.0f).withCentre(centre));

    // 2. Skirt Teeth (Knurling)
    const float lightDir = juce::MathConstants<float>::pi * 0.25f; // Top-left lighting
    for (int i = 0; i < toothCount; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * (float) i / (float) toothCount;
        const float lighting = 0.5f + 0.5f * std::cos(angle + lightDir);
        
        const auto p1 = centre.getPointOnCircumference(outerRadius * 0.73f, angle);
        const auto p2 = centre.getPointOnCircumference(outerRadius * 0.96f, angle);
        
        const float thickness = outerRadius * 0.045f;
        
        // Shadow tooth (gap)
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.drawLine({ p1.translated(1.0f, 1.5f), p2.translated(1.0f, 1.5f) }, thickness * 1.2f);
        
        // Base tooth
        juce::Colour toothColor = juce::Colour(0xff060708).interpolatedWith(juce::Colour(0xff3f4146), lighting);
        g.setColour(toothColor);
        g.drawLine({ p1, p2 }, thickness);
        
        // Specular highlight
        if (lighting > 0.6f)
        {
            float spec = (lighting - 0.6f) * 2.5f;
            g.setColour(juce::Colours::white.withAlpha(0.25f * spec));
            g.drawLine({ p1.translated(-0.5f, -0.5f), p2.translated(-0.5f, -0.5f) }, thickness * 0.3f);
        }
    }

    // 3. Cap Drop Shadow (onto skirt)
    const float capRadius = outerRadius * 0.76f;
    juce::ColourGradient capShadow(juce::Colours::black.withAlpha(0.75f), centre.x - 2.0f, centre.y + 4.0f,
                                   juce::Colours::transparentBlack, centre.x - 2.0f + capRadius * 1.15f, centre.y + 4.0f, true);
    g.setGradientFill(capShadow);
    g.fillEllipse(juce::Rectangle<float>(capRadius * 2.3f, capRadius * 2.3f).withCentre(centre.translated(-2.0f, 4.0f)));

    // 4. Cap Outer Ridge (Bevel)
    const auto capRect = juce::Rectangle<float>(capRadius * 2.0f, capRadius * 2.0f).withCentre(centre);
    juce::ColourGradient bevelGrad(juce::Colour(0xff555860), centre.x - capRadius * 0.7f, centre.y - capRadius * 0.7f,
                                   juce::Colour(0xff040506), centre.x + capRadius, centre.y, true);
    bevelGrad.addColour(0.2f, juce::Colour(0xff2d2e33));
    bevelGrad.addColour(0.8f, juce::Colour(0xff090a0b));
    g.setGradientFill(bevelGrad);
    g.fillEllipse(capRect);

    // 5. Inner Recess/Flat Face
    const float innerCapRadius = capRadius * 0.92f;
    const auto innerCapRect = juce::Rectangle<float>(innerCapRadius * 2.0f, innerCapRadius * 2.0f).withCentre(centre);
    juce::ColourGradient flatGrad(juce::Colour(0xff27292e), centre.x - innerCapRadius * 0.4f, centre.y - innerCapRadius * 0.4f,
                                  juce::Colour(0xff0a0b0d), centre.x + innerCapRadius, centre.y, true);
    g.setGradientFill(flatGrad);
    g.fillEllipse(innerCapRect);
    
    // Add realistic bakelite scratches/wear
    g.saveState();
    juce::Path capPath;
    capPath.addEllipse(innerCapRect);
    g.reduceClipRegion(capPath);
    // Use the component's absolute X/Y to seed the random so scratches don't wiggle when value changes
    uint32_t seed = (uint32_t)(getScreenPosition().getX() * 1337 + getScreenPosition().getY());
    GraphicsHelpers::drawScratches(g, innerCapRect, seed, isCompact ? 18 : 36, 0.15f);
    
    // Add some random dust/flecks
    juce::Random wear(seed + 0x48293);
    g.setColour(juce::Colour(0xff121315).withAlpha(0.6f));
    for (int i = 0; i < (isCompact ? 30 : 70); ++i)
    {
        const float x = innerCapRect.getX() + wear.nextFloat() * innerCapRect.getWidth();
        const float y = innerCapRect.getY() + wear.nextFloat() * innerCapRect.getHeight();
        g.fillRect(x, y, 0.5f + wear.nextFloat(), 0.5f + wear.nextFloat());
    }
    g.restoreState();

    // 6. Indicator Line (Deep groove with white paint)
    const auto normalised = (float) valueToProportionOfLength(getValue());
    const auto rotaryParams = getRotaryParameters();
    const float indicatorAngle = juce::jmap(normalised, rotaryParams.startAngleRadians, rotaryParams.endAngleRadians);
    const auto inner = centre.getPointOnCircumference(outerRadius * 0.22f, indicatorAngle);
    const auto outer = centre.getPointOnCircumference(outerRadius * 0.62f, indicatorAngle);
    
    // Groove shadow (cutout depth)
    g.setColour(juce::Colours::black.withAlpha(0.9f));
    g.drawLine({ inner.translated(-1.0f, -1.0f), outer.translated(-1.0f, -1.0f) }, outerRadius * 0.09f);
    
    // Groove highlight (opposite edge of cutout catching light)
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawLine({ inner.translated(1.0f, 1.0f), outer.translated(1.0f, 1.0f) }, outerRadius * 0.08f);
    
    // White paint filling
    g.setColour(juce::Colour(0xffe8e4d3));
    g.drawLine({ inner, outer }, outerRadius * 0.065f);
    
    // Paint inner shadow (dirt/depth)
    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.drawLine({ inner.translated(0.8f, 0.8f), outer.translated(0.8f, 0.8f) }, outerRadius * 0.04f);

    // Printed scale for large knobs
    if (! isCompact)
    {
        for (int i = 0; i <= 20; ++i)
        {
            const bool major = (i % 2) == 0;
            const float a = juce::degreesToRadians(225.0f + 13.5f * (float) i);
            const auto a1 = centre.getPointOnCircumference(radius * 0.64f, a);
            const auto a2 = centre.getPointOnCircumference(radius * (major ? 0.77f : 0.73f), a);
            
            // Text/Tick shadow
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.drawLine({ a1.translated(-1.0f, -1.0f), a2.translated(-1.0f, -1.0f) }, major ? 2.5f : 1.5f);
            
            g.setColour(juce::Colour(0xffddd9cb).withAlpha(0.88f));
            g.drawLine({ a1, a2 }, major ? 1.8f : 1.0f);

            if (major)
            {
                auto labelPoint = centre.getPointOnCircumference(radius * 0.88f, a);
                auto labelArea = juce::Rectangle<float>(44.0f, 22.0f).withCentre(labelPoint);
                g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(),
                                                       juce::jmax(11.0f, radius * 0.105f), juce::Font::plain)));
                // Label shadow
                g.setColour(juce::Colours::black.withAlpha(0.5f));
                g.drawFittedText(juce::String(i * 5), labelArea.translated(-1.0f, -1.0f).toNearestInt(), juce::Justification::centred, 1);
                
                g.setColour(juce::Colour(0xffddd9cb).withAlpha(0.88f));
                g.drawFittedText(juce::String(i * 5), labelArea.toNearestInt(), juce::Justification::centred, 1);
            }
        }
    }
}
