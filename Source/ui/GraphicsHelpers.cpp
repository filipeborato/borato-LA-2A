#include "GraphicsHelpers.h"

namespace GraphicsHelpers
{
juce::Image createNoiseTexture(int width, int height, uint32_t seed)
{
    juce::Image image(juce::Image::ARGB, juce::jmax(1, width), juce::jmax(1, height), true);
    juce::Random random((int64) seed);
    juce::Image::BitmapData pixels(image, juce::Image::BitmapData::writeOnly);
    for (int y = 0; y < image.getHeight(); ++y)
    {
        auto* line = pixels.getLinePointer(y);
        for (int x = 0; x < image.getWidth(); ++x)
        {
            const auto value = (uint8_t) (22 + random.nextInt(72));
            line[x * pixels.pixelStride + 0] = value;
            line[x * pixels.pixelStride + 1] = value;
            line[x * pixels.pixelStride + 2] = value;
            line[x * pixels.pixelStride + 3] = 255;
        }
    }
    return image;
}

void drawDropShadowApprox(juce::Graphics& g, juce::Rectangle<float> bounds, float radius,
                          juce::Colour colour, float offsetX, float offsetY)
{
    for (int layer = 8; layer >= 1; --layer)
    {
        const float amount = radius * (float) layer / 8.0f;
        auto area = bounds.expanded(amount * 0.45f).translated(offsetX, offsetY);
        g.setColour(colour.withMultipliedAlpha(0.035f * (9.0f - (float) layer)));
        g.fillRoundedRectangle(area, juce::jmax(2.0f, radius * 0.18f + amount * 0.12f));
    }
}

void drawRadialGradientEllipse(juce::Graphics& g, juce::Rectangle<float> bounds,
                               juce::Colour inner, juce::Colour outer, juce::Point<float> focus)
{
    const auto centre = bounds.getRelativePoint(focus.x, focus.y);
    juce::ColourGradient gradient(inner, centre.x, centre.y, outer,
                                  bounds.getRight(), bounds.getBottom(), true);
    gradient.addColour(0.55, inner.interpolatedWith(outer, 0.38f));
    g.setGradientFill(gradient);
    g.fillEllipse(bounds);
}

void drawLinearGradientRect(juce::Graphics& g, juce::Rectangle<float> bounds,
                            juce::Colour top, juce::Colour bottom)
{
    g.setGradientFill(juce::ColourGradient(top, bounds.getX(), bounds.getY(), bottom,
                                            bounds.getX(), bounds.getBottom(), false));
    g.fillRect(bounds);
}

void drawPanelTexture(juce::Graphics& g, const juce::Image& texture,
                      juce::Rectangle<float> bounds, float opacity)
{
    if (! texture.isValid())
        return;
    g.saveState();
    g.setOpacity(opacity);
    g.drawImage(texture, bounds, juce::RectanglePlacement::stretchToFit);
    g.restoreState();
}

void drawScratches(juce::Graphics& g, juce::Rectangle<float> bounds,
                   uint32_t seed, int count, float opacity)
{
    juce::Random random((int64) seed);
    for (int i = 0; i < count; ++i)
    {
        const float x = bounds.getX() + random.nextFloat() * bounds.getWidth();
        const float y = bounds.getY() + random.nextFloat() * bounds.getHeight();
        const float length = 4.0f + random.nextFloat() * 42.0f;
        const float slope = (random.nextFloat() - 0.5f) * 7.0f;
        g.setColour((i % 3 == 0 ? juce::Colours::white : juce::Colours::black)
                        .withAlpha(opacity * (0.18f + random.nextFloat() * 0.45f)));
        g.drawLine(x, y, x + length, y + slope, 0.45f + random.nextFloat() * 0.65f);
    }
}

void drawGlassReflection(juce::Graphics& g, juce::Rectangle<float> bounds,
                         float parallaxX, float parallaxY)
{
    g.saveState();
    g.reduceClipRegion(bounds.toNearestInt());
    juce::Path broad;
    const float shiftX = parallaxX * 8.0f;
    const float shiftY = parallaxY * 5.0f;
    broad.addQuadrilateral(bounds.getX() - 20.0f + shiftX, bounds.getY() + shiftY,
                           bounds.getCentreX() + 35.0f + shiftX, bounds.getY() + shiftY,
                           bounds.getCentreX() - 100.0f + shiftX, bounds.getBottom() + shiftY,
                           bounds.getX() - 45.0f + shiftX, bounds.getBottom() + shiftY);
    juce::ColourGradient glass(juce::Colours::white.withAlpha(0.14f), bounds.getX(), bounds.getY(),
                               juce::Colours::white.withAlpha(0.0f), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(glass);
    g.fillPath(broad);

    juce::Path streak;
    streak.addQuadrilateral(bounds.getCentreX() + shiftX, bounds.getY(),
                            bounds.getCentreX() + 30.0f + shiftX, bounds.getY(),
                            bounds.getCentreX() - 95.0f + shiftX, bounds.getBottom(),
                            bounds.getCentreX() - 115.0f + shiftX, bounds.getBottom());
    g.setColour(juce::Colours::white.withAlpha(0.065f));
    g.fillPath(streak);
    g.restoreState();
}

void drawEngravedText(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area,
                      float fontHeight, juce::Justification justification, float tracking)
{
    const bool isBrand = text == "BORATO" || text == "LA-2A";
    const auto family = isBrand ? juce::Font::getDefaultSansSerifFontName()
                                : juce::Font::getDefaultMonospacedFontName();
    auto font = juce::Font(juce::FontOptions(family, fontHeight,
                                             isBrand ? juce::Font::bold : juce::Font::plain))
                    .withExtraKerningFactor(tracking);
    g.setFont(font);
    g.setColour(juce::Colours::black.withAlpha(0.48f));
    g.drawFittedText(text, area.translated(1.2f, 1.5f).toNearestInt(), justification, 1);
    g.setColour(juce::Colour(0xffd8d3c5).withAlpha(0.86f));
    g.drawFittedText(text, area.toNearestInt(), justification, 1);

    juce::Random wear((int64) (0x53494c4bu + (uint32_t) text.hashCode()));
    g.saveState();
    g.reduceClipRegion(area.toNearestInt());
    g.setColour(juce::Colour(0xff111214).withAlpha(0.88f));
    const int flecks = juce::jlimit(2, 18, text.length() * 2);
    for (int i = 0; i < flecks; ++i)
    {
        const float x = area.getX() + wear.nextFloat() * area.getWidth();
        const float y = area.getY() + wear.nextFloat() * area.getHeight();
        g.fillRect(x, y, 0.8f + wear.nextFloat() * 2.2f, 0.5f + wear.nextFloat() * 1.2f);
    }
    g.restoreState();
}
}
