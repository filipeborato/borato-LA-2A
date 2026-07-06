#include "ToggleSwitchComponent.h"
#include "GraphicsHelpers.h"

ToggleSwitchComponent::ToggleSwitchComponent() : juce::Button("toggle")
{
    setOpaque(false);
    setClickingTogglesState(true);
    setTriggeredOnMouseDown(false);
}

void ToggleSwitchComponent::paintButton(juce::Graphics&, bool, bool) {}

void ToggleSwitchComponent::paint(juce::Graphics& g)
{
    const bool highlighted = isMouseOver();
    const bool down = isMouseButtonDown();
    const auto bounds = getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();
    const float radius = 25.76f;
    const float direction = getToggleState() ? -1.0f : 1.0f;

    // Hex nut path
    juce::Path hexNut;
    for (int i = 0; i < 6; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * (float) i / 6.0f
                          + juce::MathConstants<float>::pi / 6.0f;
        const auto point = centre.getPointOnCircumference(radius * 1.38f, angle);
        if (i == 0) hexNut.startNewSubPath(point); else hexNut.lineTo(point);
    }
    hexNut.closeSubPath();
    
    // Nut drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillPath(hexNut, juce::AffineTransform::translation(0.0f, 3.0f));

    // High contrast 3D metallic gradient for the nut
    juce::ColourGradient nutMetal(juce::Colour(0xffffffff), centre.x - radius * 1.3f, centre.y - radius * 1.3f,
                                  juce::Colour(0xff050505), centre.x + radius * 1.3f, centre.y + radius * 1.3f, false);
    nutMetal.addColour(0.2f, juce::Colour(0xff8c9096));
    nutMetal.addColour(0.4f, juce::Colour(0xffe2e5e8));
    nutMetal.addColour(0.6f, juce::Colour(0xff33363b));
    nutMetal.addColour(0.8f, juce::Colour(0xffa1a5aa));
    g.setGradientFill(nutMetal);
    g.fillPath(hexNut);

    // Bevel highlights for hex nut edges
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.strokePath(hexNut, juce::PathStrokeType(0.8f));
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.strokePath(hexNut, juce::PathStrokeType(0.8f), juce::AffineTransform::translation(0.5f, 0.5f));

    // Machined circular washer under the nut
    const auto washer = juce::Rectangle<float>(radius * 1.95f, radius * 1.95f).withCentre(centre);
    juce::ColourGradient washGrad(juce::Colour(0xfff0f2f5), centre.x - radius, centre.y - radius,
                                  juce::Colour(0xff222428), centre.x + radius, centre.y + radius, false);
    washGrad.addColour(0.3f, juce::Colour(0xff6a6e75));
    washGrad.addColour(0.7f, juce::Colour(0xffd1d5da));
    g.setGradientFill(washGrad);
    g.fillEllipse(washer);
    
    // Washer concentric rings
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawEllipse(washer, 0.5f);
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.drawEllipse(washer.reduced(0.5f), 0.5f);

    // Nut inner recess hole (deep pit)
    const float holeRadius = radius * 0.75f;
    const auto hole = juce::Rectangle<float>(holeRadius * 2.0f, holeRadius * 2.0f).withCentre(centre);
    
    juce::ColourGradient holeGrad(juce::Colour(0xff050505), centre.x, centre.y + holeRadius * 0.8f,
                                  juce::Colour(0xff33353a), centre.x, centre.y - holeRadius * 0.8f, false);
    g.setGradientFill(holeGrad);
    g.fillEllipse(hole);
    
    // Hole inner shadow (rim depth)
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.drawEllipse(hole, 1.5f);
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawEllipse(hole.reduced(1.5f).translated(0.0f, 1.5f), 1.0f);

    // Dynamic Lever
    const auto base = centre.translated(0.0f, direction * radius * 0.25f);
    const auto tip = centre.translated(direction * radius * 0.16f, direction * radius * 1.85f);

    // Dynamic 3D lever shadow projection
    const auto shadowBase = base.translated(-5.0f, 8.0f);
    const auto shadowTip = tip.translated(-15.0f, 18.0f);
    juce::Path leverShadow;
    leverShadow.startNewSubPath(shadowBase.x - radius * 0.2f, shadowBase.y);
    leverShadow.lineTo(shadowTip.x - radius * 0.12f, shadowTip.y);
    leverShadow.lineTo(shadowTip.x + radius * 0.12f, shadowTip.y);
    leverShadow.lineTo(shadowBase.x + radius * 0.2f, shadowBase.y);
    leverShadow.closeSubPath();
    
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillPath(leverShadow);
    g.fillEllipse(juce::Rectangle<float>(radius * 0.65f, radius * 0.65f).withCentre(shadowTip));

    // Lever Shaft
    g.saveState();
    const float angle = std::atan2(tip.x - base.x, base.y - tip.y);
    g.addTransform(juce::AffineTransform::rotation(angle).translated(base));

    const float length = radius * 1.70f;
    const float wBase = radius * 0.55f;
    const float wTip = radius * 0.38f;

    juce::Path stem;
    stem.startNewSubPath(-wBase * 0.5f, 0.0f);
    stem.lineTo(-wTip * 0.5f, -length);
    stem.lineTo(wTip * 0.5f, -length);
    stem.lineTo(wBase * 0.5f, 0.0f);
    stem.closeSubPath();

    // High contrast 3D cylindrical gradient
    juce::ColourGradient chrome(juce::Colour(0xff2a2c30), -wBase * 0.5f, 0.0f,
                                juce::Colour(0xff121418), wBase * 0.5f, 0.0f, false);
    chrome.addColour(0.2f, juce::Colour(0xffe8ebee)); // bright left spec
    chrome.addColour(0.5f, juce::Colour(0xff6a6e75)); // mid dark
    chrome.addColour(0.85f, juce::Colour(0xffbcc0c6)); // right edge bounce
    g.setGradientFill(chrome);
    g.fillPath(stem);

    // 3D Metal Ball Tip
    const float rBall = radius * 0.68f;
    const auto ballRect = juce::Rectangle<float>(rBall * 2.0f, rBall * 2.0f).withCentre({ 0.0f, -length });
    const auto ballCentre = ballRect.getCentre();
    
    juce::ColourGradient ballGrad(juce::Colour(0xffffffff), ballCentre.x - rBall * 0.3f, ballCentre.y - rBall * 0.5f,
                                  juce::Colour(0xff08090a), ballCentre.x + rBall * 0.8f, ballCentre.y + rBall * 0.8f, true);
    ballGrad.addColour(0.2f, juce::Colour(0xffd1d5da));
    ballGrad.addColour(0.6f, juce::Colour(0xff3f4348));
    ballGrad.addColour(0.9f, juce::Colour(0xff6a6e75)); // edge rim light
    g.setGradientFill(ballGrad);
    g.fillEllipse(ballRect);

    // Dynamic bright spot highlight on ball
    juce::ColourGradient brightSpot(juce::Colours::white.withAlpha(0.95f), ballCentre.x - rBall * 0.3f, ballCentre.y - rBall * 0.4f,
                                    juce::Colours::white.withAlpha(0.0f), ballCentre.x, ballCentre.y, true);
    g.setGradientFill(brightSpot);
    g.fillEllipse(juce::Rectangle<float>(rBall, rBall).withCentre(ballCentre.translated(-rBall * 0.1f, -rBall * 0.2f)));

    g.restoreState();

    if (highlighted || down)
    {
        g.setColour(juce::Colours::white.withAlpha(down ? 0.035f : 0.065f));
        g.fillEllipse(juce::Rectangle<float>(radius * 2.7f, radius * 2.7f).withCentre(centre));
    }
}
