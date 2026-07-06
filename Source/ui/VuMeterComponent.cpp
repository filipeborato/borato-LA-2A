#include "VuMeterComponent.h"
#include "GraphicsHelpers.h"
#include "RackScrew.h"

namespace
{
constexpr float designW = 530.0f;
constexpr float designH = 430.0f;
const std::array<std::pair<float, float>, 11> anchors {{
    { -20.0f, -50.0f }, { -10.0f, -29.0f }, { -7.0f, -21.0f }, { -5.0f, -14.0f },
    { -3.0f, -7.0f }, { -2.0f, -2.0f }, { -1.0f, 3.0f }, { 0.0f, 9.0f },
    { 1.0f, 18.0f }, { 2.0f, 28.0f }, { 3.0f, 38.0f }
}};
}

VuMeterComponent::VuMeterComponent()
{
    setInterceptsMouseClicks(false, false);
    startTimerHz(30);
}

VuMeterComponent::~VuMeterComponent()
{
    stopTimer();
}

void VuMeterComponent::setDb(float db)
{
    targetDb = juce::jlimit(-30.0f, 4.0f, db);
}

void VuMeterComponent::setReflectionOffset(float x, float y)
{
    reflectionX = x;
    reflectionY = y;
}

float VuMeterComponent::dbToAngle(float db)
{
    if (db <= anchors.front().first)
        return juce::jmax(-56.0f, -50.0f + (db + 20.0f) * 2.0f);
    if (db >= anchors.back().first)
        return juce::jmin(40.0f, 38.0f + (db - 3.0f) * 2.0f);
    for (size_t i = 0; i + 1 < anchors.size(); ++i)
    {
        if (db >= anchors[i].first && db <= anchors[i + 1].first)
            return juce::jmap(db, anchors[i].first, anchors[i + 1].first,
                             anchors[i].second, anchors[i + 1].second);
    }
    return 3.0f;
}

void VuMeterComponent::timerCallback()
{
    const float delta = targetDb - displayedDb;
    if (std::abs(delta) > 0.01f)
    {
        displayedDb += delta * (targetDb > displayedDb ? 0.24f : 0.10f);
        repaint();
    }
}

void VuMeterComponent::resized()
{
    rebuildStaticCache();
}

void VuMeterComponent::rebuildStaticCache()
{
    if (getWidth() <= 0 || getHeight() <= 0)
        return;
    staticCache = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    juce::Graphics g(staticCache);
    const float sx = (float) getWidth() / designW;
    const float sy = (float) getHeight() / designH;
    g.addTransform(juce::AffineTransform::scale(sx, sy));

    const auto bezel = juce::Rectangle<float>(0.0f, 0.0f, designW, designH);
    GraphicsHelpers::drawDropShadowApprox(g, bezel.reduced(6.0f), 20.0f, juce::Colours::black, -10.0f, 15.0f);
    juce::ColourGradient bezelMetal(juce::Colour(0xff45484e), 0.0f, 0.0f,
                                     juce::Colour(0xff030405), 0.0f, designH, false);
    bezelMetal.addColour(0.12, juce::Colour(0xff25272c));
    bezelMetal.addColour(0.72, juce::Colour(0xff0b0c0f));
    g.setGradientFill(bezelMetal);
    g.fillRoundedRectangle(bezel.reduced(1.0f), 28.0f);
    g.setColour(juce::Colours::white.withAlpha(0.18f));
    g.drawRoundedRectangle(bezel.reduced(4.0f), 25.0f, 2.0f);
    g.setColour(juce::Colours::black.withAlpha(0.82f));
    g.drawRoundedRectangle(bezel.reduced(11.0f), 21.0f, 5.0f);
    g.setColour(juce::Colour(0xff696c72).withAlpha(0.42f));
    g.drawRoundedRectangle(bezel.reduced(14.0f), 19.0f, 1.5f);

    const auto face = juce::Rectangle<float>(38.0f, 48.0f, 454.0f, 334.0f);
    g.setColour(juce::Colour(0xff050607));
    g.fillRoundedRectangle(face.expanded(8.0f), 20.0f);
    juce::ColourGradient paper(juce::Colour(0xfff8efe0), 215.0f, 105.0f,
                               juce::Colour(0xffbda67f), 455.0f, 382.0f, true);
    paper.addColour(0.45, juce::Colour(0xffebdcc6));
    paper.addColour(0.82, juce::Colour(0xffd4bf9c));
    g.setGradientFill(paper);
    g.fillRoundedRectangle(face, 16.0f);

    juce::Random foxing(0x2a4d4554);
    for (int i = 0; i < 42; ++i)
    {
        g.setColour(juce::Colour(0xff705b3e).withAlpha(0.08f + foxing.nextFloat() * 0.12f));
        g.fillEllipse(face.getX() + foxing.nextFloat() * face.getWidth(),
                      face.getY() + foxing.nextFloat() * face.getHeight(),
                      0.8f + foxing.nextFloat() * 2.4f, 0.8f + foxing.nextFloat() * 2.4f);
    }

    const juce::Point<float> pivot { 265.0f, 360.0f };
    const auto point = [pivot](float radius, float angleDegrees)
    {
        const float angle = juce::degreesToRadians(angleDegrees);
        return juce::Point<float>(pivot.x + radius * std::sin(angle), pivot.y - radius * std::cos(angle));
    };

    juce::Path blackArc;
    blackArc.addCentredArc(pivot.x, pivot.y, 246.0f, 246.0f, 0.0f,
                           juce::degreesToRadians(-50.0f), juce::degreesToRadians(9.0f), true);
    g.setColour(juce::Colour(0xff231f1a));
    g.strokePath(blackArc, juce::PathStrokeType(3.5f));
    juce::Path redArc;
    redArc.addCentredArc(pivot.x, pivot.y, 246.0f, 246.0f, 0.0f,
                         juce::degreesToRadians(9.0f), juce::degreesToRadians(38.0f), true);
    g.setColour(juce::Colour(0xff941e17));
    g.strokePath(redArc, juce::PathStrokeType(8.0f));

    for (const auto& [db, angle] : anchors)
    {
        g.setColour(db > 0.0f ? juce::Colour(0xff941e17) : juce::Colour(0xff231f1a));
        g.drawLine({ point(240.0f, angle), point(260.0f, angle) }, 2.4f);
        const auto labelPoint = point(278.0f, angle);
        auto area = juce::Rectangle<float>(46.0f, 22.0f).withCentre(labelPoint);
        g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 18.0f, juce::Font::bold)));
        g.drawFittedText((db > 0.0f ? "+" : "") + juce::String((int) db), area.toNearestInt(),
                         juce::Justification::centred, 1);
    }
    for (float angle : { -45.0f, -40.0f, -35.0f, -25.0f, -17.5f, -10.5f,
                         -4.5f, 0.5f, 6.0f, 13.5f, 23.0f, 33.0f })
    {
        g.setColour(angle > 9.0f ? juce::Colour(0xff941e17) : juce::Colour(0xff231f1a));
        g.drawLine({ point(240.0f, angle), point(251.0f, angle) }, 1.2f);
    }
    g.setColour(juce::Colour(0xff231f1a));
    g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 28.0f, juce::Font::bold)));
    g.drawText("DB", juce::Rectangle<int>(225, 235, 80, 40), juce::Justification::centred);

    for (const auto& item : std::array<std::tuple<float, float, float>, 4> {{
             { 32.0f, 32.0f, 0.25f }, { 488.0f, 32.0f, -0.75f },
             { 32.0f, 398.0f, 1.15f }, { 488.0f, 398.0f, -0.30f } }})
        RackScrew::draw(g, { std::get<0>(item), std::get<1>(item) }, 15.0f, std::get<2>(item));
}

void VuMeterComponent::paint(juce::Graphics& g)
{
    g.drawImageAt(staticCache, 0, 0);
    const float sx = (float) getWidth() / designW;
    const float sy = (float) getHeight() / designH;
    g.saveState();
    g.addTransform(juce::AffineTransform::scale(sx, sy));
    const juce::Point<float> pivot { 265.0f, 360.0f };
    const float angle = juce::degreesToRadians(dbToAngle(displayedDb));
    const auto tip = juce::Point<float>(pivot.x + 232.0f * std::sin(angle), pivot.y - 232.0f * std::cos(angle));
    g.setColour(juce::Colours::black.withAlpha(0.30f));
    g.drawLine({ pivot.translated(5.0f, 4.0f), tip.translated(5.0f, 4.0f) }, 4.0f);
    g.setColour(juce::Colour(0xff100e0b));
    g.drawLine({ pivot, tip }, 2.8f);

    juce::Path housing;
    housing.startNewSubPath(200.0f, 382.0f);
    housing.lineTo(200.0f, 355.0f);
    housing.cubicTo(203.0f, 316.0f, 327.0f, 316.0f, 330.0f, 355.0f);
    housing.lineTo(330.0f, 382.0f);
    housing.closeSubPath();
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff777b83), 265.0f, 320.0f,
                                           juce::Colour(0xff111216), 265.0f, 382.0f, false));
    g.fillPath(housing);
    g.setColour(juce::Colours::white.withAlpha(0.18f));
    g.strokePath(housing, juce::PathStrokeType(1.2f));
    RackScrew::draw(g, { 222.0f, 365.0f }, 6.0f, 0.60f);
    RackScrew::draw(g, { 308.0f, 365.0f }, 6.0f, -0.90f);
    GraphicsHelpers::drawRadialGradientEllipse(g,
        juce::Rectangle<float>(28.0f, 28.0f).withCentre({ 265.0f, 366.0f }),
        juce::Colour(0xffa8aab0), juce::Colour(0xff24262b));
    GraphicsHelpers::drawGlassReflection(g, { 38.0f, 48.0f, 454.0f, 334.0f }, reflectionX, reflectionY);
    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.drawRoundedRectangle({ 38.0f, 48.0f, 454.0f, 334.0f }, 16.0f, 2.0f);
    g.restoreState();
}
