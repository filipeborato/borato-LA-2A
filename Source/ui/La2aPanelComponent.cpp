#include "La2aPanelComponent.h"
#include "GraphicsHelpers.h"
#include "RackScrew.h"

namespace
{
void drawRackSlot(juce::Graphics& g, juce::Rectangle<float> slot)
{
    g.setColour(juce::Colours::black.withAlpha(0.70f));
    g.fillRoundedRectangle(slot.translated(1.5f, 3.0f).expanded(1.5f), slot.getHeight() * 0.5f);
    juce::ColourGradient recess(juce::Colour(0xff020304), slot.getX(), slot.getY(),
                                juce::Colour(0xff303238), slot.getX(), slot.getBottom(), false);
    g.setGradientFill(recess);
    g.fillRoundedRectangle(slot, slot.getHeight() * 0.5f);
    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.drawRoundedRectangle(slot.reduced(1.0f), slot.getHeight() * 0.45f, 1.0f);
}
}

La2aPanelComponent::La2aPanelComponent(BoratoLA2AAudioProcessor& p)
    : processor(p), gainKnob(false), peakKnob(false)
{
    gainKnob.setRange(0.0, 100.0, 0.1);
    peakKnob.setRange(0.0, 100.0, 0.1);
    meterSelector.setRange(0.0, 2.0, 1.0);
    r37Knob.setRange(0.0, 1.0, 0.001);
    for (auto* child : std::array<juce::Component*, 8> {
             &meter, &gainKnob, &peakKnob, &meterSelector, &modeSwitch, &powerSwitch, &jewelLight, &r37Knob })
        addAndMakeVisible(*child);

    jewelLight.setInterceptsMouseClicks(false, false);

    auto& state = processor.apvts;
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, Params::gain, gainKnob);
    peakAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, Params::peakReduction, peakKnob);
    meterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, Params::meterMode, meterSelector);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, Params::mode, modeSwitch);
    powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, Params::power, powerSwitch);
    r37Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, Params::r37PreEmphasis, r37Knob);

    addMouseListener(this, true);
    startTimerHz(30);
}

La2aPanelComponent::~La2aPanelComponent()
{
    stopTimer();
    removeMouseListener(this);
}

juce::Rectangle<float> La2aPanelComponent::getDesignArea() const
{
    const float scale = juce::jmin((float) getWidth() / designWidth, (float) getHeight() / designHeight);
    const float width = designWidth * scale;
    const float height = designHeight * scale;
    return { ((float) getWidth() - width) * 0.5f, ((float) getHeight() - height) * 0.5f, width, height };
}

juce::AffineTransform La2aPanelComponent::designTransform() const
{
    const auto area = getDesignArea();
    return juce::AffineTransform::scale(area.getWidth() / designWidth, area.getHeight() / designHeight)
        .translated(area.getX(), area.getY());
}

juce::Rectangle<int> La2aPanelComponent::mapRect(juce::Rectangle<float> rect) const
{
    return rect.transformedBy(designTransform()).toNearestInt();
}

void La2aPanelComponent::resized()
{
    meter.setBounds(mapRect({ 460.0f, 210.0f, 530.0f, 430.0f }));
    gainKnob.setBounds(mapRect({ 82.0f, 500.0f, 360.0f, 360.0f }));
    peakKnob.setBounds(mapRect({ 790.0f, 700.0f, 290.0f, 280.0f }));
    meterSelector.setBounds(mapRect({ 540.0f, 818.0f, 104.0f, 104.0f }));
    modeSwitch.setBounds(mapRect({ 1153.0f, 289.5f, 130.0f, 169.0f }));
    jewelLight.setBounds(mapRect({ 1192.0f, 508.0f, 240.0f, 240.0f }));
    powerSwitch.setBounds(mapRect({ 1247.0f, 749.5f, 130.0f, 163.0f }));
    r37Knob.setBounds(mapRect({ 1108.0f, 712.0f, 40.0f, 40.0f }));
    rebuildBackgroundCache();
}

void La2aPanelComponent::rebuildBackgroundCache()
{
    if (getWidth() <= 0 || getHeight() <= 0)
        return;
    backgroundCache = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    const auto panelPixelBounds = mapRect({ 20.0f, 70.0f, 1400.0f, 940.0f });
    panelNoise = GraphicsHelpers::createNoiseTexture(juce::jmax(1, panelPixelBounds.getWidth() / 2),
                                                      juce::jmax(1, panelPixelBounds.getHeight() / 2));
    juce::Graphics g(backgroundCache);
    drawStaticPanel(g);
}

void La2aPanelComponent::drawStaticPanel(juce::Graphics& g)
{
    const auto full = getLocalBounds().toFloat();
    juce::ColourGradient studioBg(juce::Colour(0xff2a2a32), full.getCentreX(), full.getHeight() * 0.42f,
                                  juce::Colour(0xff08080a), full.getCentreX() + full.getWidth() * 0.85f, full.getHeight() * 0.42f, true);
    studioBg.addColour(0.55, juce::Colour(0xff15151a));
    g.setGradientFill(studioBg);
    g.fillRect(full);

    g.saveState();
    g.addTransform(designTransform());
    const juce::Rectangle<float> panel { 20.0f, 70.0f, 1400.0f, 940.0f };
    GraphicsHelpers::drawDropShadowApprox(g, panel, 28.0f, juce::Colours::black.withAlpha(0.8f), 0.0f, 22.0f);
    juce::ColourGradient paint(juce::Colour(0xff55585f), 0.0f, panel.getY(),
                               juce::Colour(0xff2d3034), 0.0f, panel.getBottom(), false);
    paint.addColour(0.2, juce::Colour(0xff4c4f56));
    paint.addColour(0.6, juce::Colour(0xff3d4046));
    g.setGradientFill(paint);
    g.fillRoundedRectangle(panel, 12.0f);
    g.setColour(juce::Colours::white.withAlpha(0.11f));
    g.drawRoundedRectangle(panel.reduced(2.0f), 10.0f, 2.0f);
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.drawRoundedRectangle(panel.reduced(7.0f), 8.0f, 3.0f);
    g.restoreState();

    GraphicsHelpers::drawPanelTexture(g, panelNoise, mapRect({ 20.0f, 70.0f, 1400.0f, 940.0f }).toFloat(), 0.16f);

    g.saveState();
    g.addTransform(designTransform());
    GraphicsHelpers::drawScratches(g, panel.reduced(32.0f), 0x53435254u, 48, 0.25f);

    juce::Random surfaceWear(0x4c413241);
    for (int i = 0; i < 260; ++i)
    {
        const auto colour = (i % 9 == 0 ? juce::Colour(0xff70533d) : juce::Colour(0xffa3a09a))
                                .withAlpha(0.035f + surfaceWear.nextFloat() * 0.075f);
        g.setColour(colour);
        const float size = 0.35f + surfaceWear.nextFloat() * 1.45f;
        g.fillEllipse(panel.getX() + 20.0f + surfaceWear.nextFloat() * (panel.getWidth() - 40.0f),
                      panel.getY() + 16.0f + surfaceWear.nextFloat() * (panel.getHeight() - 32.0f),
                      size, size * (0.55f + surfaceWear.nextFloat()));
    }

    drawRackSlot(g, { 18.0f, 255.0f, 46.0f, 19.0f });
    drawRackSlot(g, { 18.0f, 836.0f, 46.0f, 19.0f });
    drawRackSlot(g, { 1376.0f, 255.0f, 46.0f, 19.0f });
    drawRackSlot(g, { 1376.0f, 836.0f, 46.0f, 19.0f });

    GraphicsHelpers::drawEngravedText(g, "BORATO", { 165.0f, 180.0f, 290.0f, 58.0f }, 46.0f,
                                      juce::Justification::centredLeft, 0.08f);
    GraphicsHelpers::drawEngravedText(g, "COMPANY", { 168.0f, 232.0f, 290.0f, 38.0f }, 27.0f,
                                      juce::Justification::centredLeft, 0.20f);
    GraphicsHelpers::drawEngravedText(g, "LEVELING AMPLIFIER", { 168.0f, 305.0f, 300.0f, 34.0f }, 20.0f,
                                      juce::Justification::centredLeft, 0.10f);
    GraphicsHelpers::drawEngravedText(g, "MODEL LA-2A", { 168.0f, 340.0f, 280.0f, 34.0f }, 20.0f,
                                      juce::Justification::centredLeft, 0.12f);
    GraphicsHelpers::drawEngravedText(g, "LA-2A", { 1120.0f, 190.0f, 255.0f, 76.0f }, 56.0f);
    GraphicsHelpers::drawEngravedText(g, "GAIN", { 170.0f, 445.0f, 184.0f, 45.0f }, 27.0f, juce::Justification::centred, 0.18f);
    GraphicsHelpers::drawEngravedText(g, "PEAK REDUCTION", { 790.0f, 648.0f, 290.0f, 48.0f }, 26.0f, juce::Justification::centred, 0.12f);
    GraphicsHelpers::drawEngravedText(g, "METER", { 520.0f, 710.0f, 144.0f, 40.0f }, 23.0f, juce::Justification::centred, 0.16f);
    GraphicsHelpers::drawEngravedText(g, "+10", { 468.0f, 782.0f, 64.0f, 34.0f }, 21.0f);
    GraphicsHelpers::drawEngravedText(g, "+4", { 560.0f, 750.0f, 64.0f, 34.0f }, 21.0f);
    GraphicsHelpers::drawEngravedText(g, "GR", { 646.0f, 782.0f, 64.0f, 34.0f }, 21.0f);
    GraphicsHelpers::drawEngravedText(g, "LIMIT", { 1162.0f, 286.0f, 112.0f, 36.0f }, 21.0f, juce::Justification::centred, 0.10f);
    GraphicsHelpers::drawEngravedText(g, "COMPRESS", { 1145.0f, 426.0f, 146.0f, 36.0f }, 20.0f, juce::Justification::centred, 0.08f);
    GraphicsHelpers::drawEngravedText(g, "ON", { 1270.0f, 720.0f, 84.0f, 36.0f }, 23.0f, juce::Justification::centred, 0.12f);
    GraphicsHelpers::drawEngravedText(g, "OFF", { 1264.0f, 906.0f, 96.0f, 36.0f }, 23.0f, juce::Justification::centred, 0.12f);

    for (const auto& screw : std::array<std::tuple<float, float, float>, 4> {{
             { 68.0f, 178.0f, 0.40f }, { 1372.0f, 178.0f, -0.70f },
             { 66.0f, 916.0f, 1.15f }, { 1372.0f, 916.0f, 0.14f } }})
    {
        const juce::Point<float> centre { std::get<0>(screw), std::get<1>(screw) };
        const auto socketArea = juce::Rectangle<float>(58.0f, 58.0f).withCentre(centre);
        // Recessed metal shadow/gradient for 3D depth
        juce::ColourGradient socketGrad(juce::Colour(0xff090b0d), centre.x - 29.0f, centre.y - 29.0f,
                                         juce::Colour(0xff292c32), centre.x + 29.0f, centre.y + 29.0f, false);
        socketGrad.addColour(0.35, juce::Colour(0xff121316));
        socketGrad.addColour(0.75, juce::Colour(0xff1c1d22));
        g.setGradientFill(socketGrad);
        g.fillEllipse(socketArea);
        // Inner bevel highlight
        g.setColour(juce::Colour(0xff07080a));
        g.drawEllipse(socketArea, 1.5f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawEllipse(socketArea.reduced(1.0f), 0.8f);
        RackScrew::draw(g, centre, 20.0f, std::get<2>(screw));
    }
    // O parafuso do R37 agora é o componente interativo r37Knob (ver
    // resized()); aqui só o rótulo gravado permanece no fundo estático.
    GraphicsHelpers::drawEngravedText(g, "R37", { 1092.0f, 665.0f, 72.0f, 32.0f }, 18.0f);

    // Static base drop shadows for Jewel Light and Switches to prevent component clipping bounds squares
    {
        const auto centre = juce::Point<float>(1312.0f, 628.0f);
        const float radius = 32.24f;
        const float shadowBlur = radius * 0.95f;
        const float dx = -4.0f;
        const float dy = 6.0f;
        const float shadowRadius = radius * 2.7f + shadowBlur;
        const auto shadowCentre = centre.translated(dx, dy);

        juce::ColourGradient shadowGrad(
            juce::Colours::black.withAlpha(0.45f), shadowCentre.x, shadowCentre.y,
            juce::Colours::transparentBlack, shadowCentre.x + shadowRadius, shadowCentre.y,
            true);
        shadowGrad.addColour(0.35, juce::Colours::black.withAlpha(0.22f));
        shadowGrad.addColour(0.70, juce::Colours::black.withAlpha(0.06f));
        g.setGradientFill(shadowGrad);
        g.fillEllipse(juce::Rectangle<float>(shadowRadius * 2.0f, shadowRadius * 2.0f).withCentre(shadowCentre));

        // Contact shadow
        const float contactRadius = radius * 2.75f;
        const auto contactCentre = centre.translated(dx * 0.25f, dy * 0.25f);
        juce::ColourGradient contactGrad(
            juce::Colours::black.withAlpha(0.55f), contactCentre.x, contactCentre.y,
            juce::Colours::transparentBlack, contactCentre.x + contactRadius, contactCentre.y,
            true);
        g.setGradientFill(contactGrad);
        g.fillEllipse(juce::Rectangle<float>(contactRadius * 2.0f, contactRadius * 2.0f).withCentre(contactCentre));
    }

    auto drawSwitchBaseShadow = [&](juce::Graphics& gr, juce::Point<float> centrePoint)
    {
        const float radius = 25.76f;
        const float shadowBlur = radius * 1.10f;
        const float dx = -5.0f;
        const float dy = 8.0f;
        const float shadowRadius = radius * 1.38f + shadowBlur;
        const auto shadowCentre = centrePoint.translated(dx, dy);

        juce::ColourGradient shadowGrad(
            juce::Colours::black.withAlpha(0.55f), shadowCentre.x, shadowCentre.y,
            juce::Colours::transparentBlack, shadowCentre.x + shadowRadius, shadowCentre.y,
            true);
        shadowGrad.addColour(0.28, juce::Colours::black.withAlpha(0.35f));
        shadowGrad.addColour(0.65, juce::Colours::black.withAlpha(0.08f));
        gr.setGradientFill(shadowGrad);
        gr.fillEllipse(juce::Rectangle<float>(shadowRadius * 2.0f, shadowRadius * 2.0f).withCentre(shadowCentre));

        // Contact shadow
        const float contactRadius = radius * 1.42f;
        const auto contactCentre = centrePoint.translated(dx * 0.25f, dy * 0.25f);
        juce::ColourGradient contactGrad(
            juce::Colours::black.withAlpha(0.60f), contactCentre.x, contactCentre.y,
            juce::Colours::transparentBlack, contactCentre.x + contactRadius, contactCentre.y,
            true);
        contactGrad.addColour(0.70, juce::Colours::black.withAlpha(0.20f));
        gr.setGradientFill(contactGrad);
        gr.fillEllipse(juce::Rectangle<float>(contactRadius * 2.0f, contactRadius * 2.0f).withCentre(contactCentre));
    };

    drawSwitchBaseShadow(g, { 1218.0f, 374.0f });
    drawSwitchBaseShadow(g, { 1312.0f, 831.0f });

    g.restoreState();
}

void La2aPanelComponent::paint(juce::Graphics& g)
{
    g.drawImageAt(backgroundCache, 0, 0);
}

void La2aPanelComponent::paintOverChildren(juce::Graphics& g)
{
    const auto panel = mapRect({ 20.0f, 70.0f, 1400.0f, 940.0f }).toFloat();
    const auto lightCentre = panel.getRelativePoint(0.4f + tiltX * 0.025f, 0.3f + tiltY * 0.02f);
    const float radius = panel.getWidth() * 0.95f;
    juce::ColourGradient sheen(juce::Colours::white.withAlpha(0.14f), lightCentre.x, lightCentre.y,
                               juce::Colours::black.withAlpha(0.55f), lightCentre.x + radius, lightCentre.y + radius, true);
    sheen.addColour(0.45, juce::Colours::white.withAlpha(0.02f));
    sheen.addColour(0.75, juce::Colours::black.withAlpha(0.22f));
    g.setGradientFill(sheen);
    g.fillRoundedRectangle(panel, 12.0f);

    juce::ColourGradient vignette(juce::Colours::transparentBlack, panel.getCentreX(), panel.getCentreY(),
                                  juce::Colours::black.withAlpha(0.5f), panel.getCentreX() + panel.getWidth() * 0.74f, panel.getCentreY(), true);
    vignette.addColour(0.6, juce::Colours::transparentBlack);
    g.setGradientFill(vignette);
    g.fillRoundedRectangle(panel, 12.0f);
}

void La2aPanelComponent::timerCallback()
{
    const auto* meterMode = processor.apvts.getRawParameterValue(Params::meterMode);
    const auto* power = processor.apvts.getRawParameterValue(Params::power);
    const bool powered = power != nullptr && power->load(std::memory_order_relaxed) >= 0.5f;
    const int selected = meterMode != nullptr ? juce::roundToInt(meterMode->load(std::memory_order_relaxed)) : 2;
    float db = -30.0f;
    if (powered)
    {
        if (selected == 2)
            db = -processor.getGainReductionDb();
        else
            db = processor.getMeterDb() - (selected == 0 ? 6.0f : 0.0f);
    }
    meter.setDb(db);
    jewelLight.setOn(powered);

    const float nextX = tiltX + (targetTiltX - tiltX) * 0.12f;
    const float nextY = tiltY + (targetTiltY - tiltY) * 0.12f;
    if (std::abs(nextX - tiltX) > 0.0002f || std::abs(nextY - tiltY) > 0.0002f)
    {
        tiltX = nextX;
        tiltY = nextY;
        meter.setReflectionOffset(tiltX, tiltY);
        repaint();
    }

    // Capture screenshot on frame 15 for automated visual verification
    static int frameCount = 0;
    if (frameCount >= 0 && ++frameCount >= 15)
    {
        const auto bounds = getLocalBounds();
        if (! bounds.isEmpty())
        {
            frameCount = -1; // Stop capturing
            const auto file = juce::File ("C:/Users/filipe/projects/borato-LA-2A/screenshot.png");
            const auto snapshot = createComponentSnapshot (bounds);
            juce::PNGImageFormat png;
            if (auto outStream = std::unique_ptr<juce::FileOutputStream> (file.createOutputStream()))
            {
                png.writeImageToStream (snapshot, *outStream);
            }
        }
    }
}

void La2aPanelComponent::mouseMove(const juce::MouseEvent& event)
{
    if (! parallaxEnabled)
        return;
    const auto area = getDesignArea();
    targetTiltX = juce::jlimit(-1.0f, 1.0f, (event.position.x - area.getCentreX()) / (area.getWidth() * 0.5f));
    targetTiltY = juce::jlimit(-1.0f, 1.0f, (event.position.y - area.getCentreY()) / (area.getHeight() * 0.5f));
}

void La2aPanelComponent::mouseExit(const juce::MouseEvent&)
{
    targetTiltX = 0.0f;
    targetTiltY = 0.0f;
}
