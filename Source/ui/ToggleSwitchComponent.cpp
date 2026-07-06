#include "ToggleSwitchComponent.h"
#include "GraphicsHelpers.h"

/*
    Chave alavanca (bat-handle) sobre porca sextavada, envelhecida para casar
    com o resto do painel. Construção: arruela usinada -> porca sextavada com
    facetas chanfradas -> colar rosqueado -> soquete escuro -> alavanca em
    tronco de cone + corpo de cilindro + tampa arredondada vista quase de
    frente. "On" é o espelho vertical exato de "off" (mesma haste curta/larga,
    só invertida) — ver referência OpenClipart "machine switch".

    Realismo aplicado (ver Assets/togle-metalic-switch.svg como referência de
    construção vetorial de metal): poucos tons por faceta em vez de um
    gradiente arco-íris ao redor de toda a porca, um único "catch-light"
    alongado por peça (não um brilho radial simétrico), sombra dupla (difusa +
    contato), reflexo tipo ambiente (céu/horizonte/chão) no cromo em vez de
    gradiente radial genérico, escurecimento tipo fresnel na borda da tampa,
    e a mesma camada de desgaste (ruído + riscos + pátina) usada no resto do
    painel, clipada à forma do metal.
*/

namespace
{
constexpr float lightAngle = -juce::MathConstants<float>::pi * 0.25f; // luz vinda do topo-esquerda

// Tons de níquel envelhecido (mesma paleta do JewelLightComponent) em vez de
// preto/branco puros — evita o "aço de fábrica" que destoa do painel gasto.
juce::Colour facetShade(float faceNormalAngle)
{
    const float alignment = 0.5f + 0.5f * std::cos(faceNormalAngle - lightAngle);
    const float t = std::pow(alignment, 1.6f);
    return juce::Colour(0xff2a2c30).interpolatedWith(juce::Colour(0xffcac5b8), t);
}

float facetLighting(float faceNormalAngle)
{
    return 0.5f + 0.5f * std::cos(faceNormalAngle - lightAngle);
}

// Escovado anisotrópico: arcos concêntricos aleatórios de baixa opacidade,
// simulando a usinagem em torno (lathe brushing) da arruela/topo da porca.
void drawLatheBrushing(juce::Graphics& g, juce::Point<float> centre, float rMin, float rMax,
                       int count, uint32_t seed)
{
    juce::Random rng((juce::int64) seed);
    for (int i = 0; i < count; ++i)
    {
        const float r = rMin + rng.nextFloat() * (rMax - rMin);
        const float a0 = rng.nextFloat() * juce::MathConstants<float>::twoPi;
        const float sweep = juce::MathConstants<float>::pi * (0.12f + rng.nextFloat() * 0.45f);
        juce::Path arc;
        arc.addCentredArc(centre.x, centre.y, r, r, 0.0f, a0, a0 + sweep, true);
        const bool light = rng.nextBool();
        g.setColour((light ? juce::Colours::white : juce::Colours::black)
                        .withAlpha(0.04f + rng.nextFloat() * 0.05f));
        g.strokePath(arc, juce::PathStrokeType(0.5f + rng.nextFloat() * 0.6f));
    }
}

// Brilho especular alongado ("catch-light"), não um blob radial simétrico —
// é assim que o reflexo de metal curvo aparece em ilustrações vetoriais
// fotorrealistas (ver os "gradient_N" da referência SVG).
void drawSpecularStreak(juce::Graphics& g, juce::Point<float> centre, float length, float thickness,
                        float angleRad, float alpha)
{
    g.saveState();
    g.addTransform(juce::AffineTransform::rotation(angleRad).translated(centre));
    juce::ColourGradient grad(juce::Colour(0xfff4efe2).withAlpha(alpha), 0.0f, -length * 0.5f,
                              juce::Colour(0xfff4efe2).withAlpha(0.0f), 0.0f, length * 0.5f, true);
    g.setGradientFill(grad);
    g.fillEllipse(juce::Rectangle<float>(thickness, length).withCentre({ 0.0f, 0.0f }));
    g.restoreState();
}
} // namespace

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
    const bool up = getToggleState();

    const auto bounds = getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();
    const float radius = juce::jmin(bounds.getWidth() * 0.176f, bounds.getHeight() * 0.1355f);

    // ---- Sombra dupla projetada no painel: difusa larga + contato apertado ----
    {
        const auto softShadow = juce::Rectangle<float>(radius * 3.6f, radius * 3.0f)
                                    .withCentre(centre.translated(radius * 0.22f, radius * 0.30f));
        juce::ColourGradient soft(juce::Colours::black.withAlpha(0.30f), softShadow.getCentre().x, softShadow.getCentre().y,
                                  juce::Colours::transparentBlack, softShadow.getRight(), softShadow.getCentre().y, true);
        g.setGradientFill(soft);
        g.fillEllipse(softShadow);

        const auto contact = juce::Rectangle<float>(radius * 3.05f, radius * 2.55f)
                                 .withCentre(centre.translated(radius * 0.09f, radius * 0.12f));
        juce::ColourGradient tight(juce::Colours::black.withAlpha(0.42f), contact.getCentre().x, contact.getCentre().y,
                                   juce::Colours::transparentBlack, contact.getRight(), contact.getCentre().y, true);
        g.setGradientFill(tight);
        g.fillEllipse(contact);
    }

    // ---- Arruela circular usinada, tom de níquel envelhecido ----
    const auto washer = juce::Rectangle<float>(radius * 3.16f, radius * 3.16f).withCentre(centre);
    {
        juce::ColourGradient washGrad(juce::Colour(0xffc6c2b7), centre.x - radius * 1.3f, centre.y - radius * 1.3f,
                                      juce::Colour(0xff17181b), centre.x + radius * 1.3f, centre.y + radius * 1.3f, false);
        washGrad.addColour(0.20f, juce::Colour(0xffdedacd));
        washGrad.addColour(0.42f, juce::Colour(0xff6f716d));
        washGrad.addColour(0.68f, juce::Colour(0xff292b30));
        washGrad.addColour(0.88f, juce::Colour(0xffa8a49a));
        g.setGradientFill(washGrad);
        g.fillEllipse(washer);

        g.saveState();
        juce::Path washClip;
        washClip.addEllipse(washer);
        g.reduceClipRegion(washClip);
        drawLatheBrushing(g, centre, radius * 1.15f, radius * 1.58f, 26, 0x574c4154u);
        g.restoreState();

        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.drawEllipse(washer, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.22f));
        g.drawEllipse(washer.reduced(1.2f), 0.8f);
    }

    // ---- Porca sextavada ----
    const float hexOuterR = radius * 1.42f;
    const float hexInnerR = hexOuterR * 0.80f;

    std::array<juce::Point<float>, 6> outerPts, innerPts;
    for (int i = 0; i < 6; ++i)
    {
        const float vertexAngle = juce::MathConstants<float>::twoPi * (float) i / 6.0f
                                + juce::MathConstants<float>::pi / 6.0f;
        outerPts[(size_t) i] = centre.getPointOnCircumference(hexOuterR, vertexAngle);
        innerPts[(size_t) i] = centre.getPointOnCircumference(hexInnerR, vertexAngle);
    }

    // Sombra da porca projetada na arruela (3 passes translúcidos decrescentes)
    {
        juce::Path hexOutline;
        hexOutline.startNewSubPath(outerPts[0]);
        for (int i = 1; i < 6; ++i)
            hexOutline.lineTo(outerPts[(size_t) i]);
        hexOutline.closeSubPath();

        for (int pass = 0; pass < 3; ++pass)
        {
            const float off = (float) (pass + 1) * radius * 0.045f;
            g.setColour(juce::Colours::black.withAlpha(0.16f - (float) pass * 0.045f));
            g.fillPath(hexOutline, juce::AffineTransform::translation(off, off));
        }
    }

    // Facetas do chanfro: tom base discreto (pouco contraste) + UM catch-light
    // na faceta mais alinhada à luz, em vez de um degradê uniforme nas 6.
    int litFacet = 0;
    float bestAlign = -2.0f;
    for (int i = 0; i < 6; ++i)
    {
        const float faceNormal = juce::MathConstants<float>::twoPi * ((float) i + 0.5f) / 6.0f
                               + juce::MathConstants<float>::pi / 6.0f;
        const float align = facetLighting(faceNormal);
        if (align > bestAlign) { bestAlign = align; litFacet = i; }
    }

    for (int i = 0; i < 6; ++i)
    {
        const int j = (i + 1) % 6;
        const float faceNormal = juce::MathConstants<float>::twoPi * ((float) i + 0.5f) / 6.0f
                               + juce::MathConstants<float>::pi / 6.0f;

        juce::Path face;
        face.startNewSubPath(outerPts[(size_t) i]);
        face.lineTo(outerPts[(size_t) j]);
        face.lineTo(innerPts[(size_t) j]);
        face.lineTo(innerPts[(size_t) i]);
        face.closeSubPath();

        juce::ColourGradient faceGrad(facetShade(faceNormal), outerPts[(size_t) i].x, outerPts[(size_t) i].y,
                                      facetShade(faceNormal).darker(0.35f), innerPts[(size_t) i].x, innerPts[(size_t) i].y, false);
        g.setGradientFill(faceGrad);
        g.fillPath(face);

        // Aresta entre facetas: sutil, clara ou escura conforme a direção da luz.
        const float edgeAlign = facetLighting(juce::MathConstants<float>::twoPi * (float) i / 6.0f + juce::MathConstants<float>::pi / 6.0f);
        g.setColour((edgeAlign > 0.5f ? juce::Colours::white : juce::Colours::black).withAlpha(0.16f));
        g.drawLine({ outerPts[(size_t) i], innerPts[(size_t) i] }, 0.6f);
    }

    // Catch-light único, alongado, só na faceta mais bem iluminada.
    {
        const float faceNormal = juce::MathConstants<float>::twoPi * ((float) litFacet + 0.5f) / 6.0f
                               + juce::MathConstants<float>::pi / 6.0f;
        const auto mid = (outerPts[(size_t) litFacet] + outerPts[(size_t) ((litFacet + 1) % 6)]) * 0.5f;
        const auto streakCentre = mid.translated(-std::sin(faceNormal) * hexOuterR * 0.08f,
                                                  std::cos(faceNormal) * hexOuterR * 0.08f);
        drawSpecularStreak(g, streakCentre, hexOuterR * 0.42f, hexOuterR * 0.16f, -faceNormal, 0.55f);
    }

    // Contorno externo: aresta clara/escura em vez de traço preto uniforme.
    for (int i = 0; i < 6; ++i)
    {
        const int j = (i + 1) % 6;
        const float faceNormal = juce::MathConstants<float>::twoPi * ((float) i + 0.5f) / 6.0f
                               + juce::MathConstants<float>::pi / 6.0f;
        const float align = facetLighting(faceNormal);
        g.setColour(juce::Colours::black.withAlpha(0.32f + (1.0f - align) * 0.25f));
        g.drawLine({ outerPts[(size_t) i], outerPts[(size_t) j] }, 1.0f);
    }

    // Topo plano da porca (hexágono interno) — metal escovado com AO na borda
    {
        juce::Path topFace;
        topFace.startNewSubPath(innerPts[0]);
        for (int i = 1; i < 6; ++i)
            topFace.lineTo(innerPts[(size_t) i]);
        topFace.closeSubPath();

        juce::ColourGradient topGrad(juce::Colour(0xffcac5b8), centre.x - hexInnerR, centre.y - hexInnerR,
                                     juce::Colour(0xff5c5f63), centre.x + hexInnerR, centre.y + hexInnerR, false);
        topGrad.addColour(0.55f, juce::Colour(0xffa5a099));
        g.setGradientFill(topGrad);
        g.fillPath(topFace);

        g.saveState();
        g.reduceClipRegion(topFace);
        drawLatheBrushing(g, centre, hexInnerR * 0.1f, hexInnerR * 0.95f, 16, 0x4a575344u);
        juce::ColourGradient ao(juce::Colours::black.withAlpha(0.30f), centre.x, centre.y,
                                juce::Colours::transparentBlack, centre.x, centre.y - hexInnerR, true);
        g.setGradientFill(ao);
        g.fillPath(topFace);
        g.restoreState();

        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.strokePath(topFace, juce::PathStrokeType(0.7f));
    }

    // ---- Colar rosqueado ----
    const float collarR = radius * 0.88f;
    {
        const auto collar = juce::Rectangle<float>(collarR * 2.0f, collarR * 2.0f).withCentre(centre);
        juce::ColourGradient collarGrad(juce::Colour(0xffd6d2c5), centre.x - collarR * 0.7f, centre.y - collarR * 0.7f,
                                        juce::Colour(0xff3d4045), centre.x + collarR * 0.7f, centre.y + collarR * 0.7f, false);
        collarGrad.addColour(0.5f, juce::Colour(0xff8f8b83));
        g.setGradientFill(collarGrad);
        g.fillEllipse(collar);

        g.setColour(juce::Colours::black.withAlpha(0.22f));
        g.drawEllipse(collar.reduced(collarR * 0.14f), 0.7f);
        g.drawEllipse(collar.reduced(collarR * 0.28f), 0.7f);
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawEllipse(collar, 0.9f);
    }

    // ---- Soquete escuro ----
    const float socketR = radius * 0.55f;
    {
        const auto socket = juce::Rectangle<float>(socketR * 2.0f, socketR * 2.0f).withCentre(centre);
        juce::ColourGradient socketGrad(juce::Colour(0xff020303), centre.x, centre.y - socketR * 0.4f,
                                        juce::Colour(0xff2e3136), centre.x, centre.y + socketR * 0.9f, true);
        g.setGradientFill(socketGrad);
        g.fillEllipse(socket);

        g.setColour(juce::Colours::black.withAlpha(0.85f));
        g.drawEllipse(socket, 1.4f);
        g.setColour(juce::Colours::white.withAlpha(0.20f));
        g.drawEllipse(socket.reduced(1.4f).translated(0.0f, 1.2f), 0.9f);
    }

    // ---- Alavanca: tronco de cone -> crescente do cilindro -> tampa ----
    // "on" é o espelho vertical exato de "off".
    const float tilt = up ? -1.0f : 1.0f;
    const auto base = centre;
    const auto tip = centre.translated(-tilt * 0.05f * radius, tilt * 0.68f * radius);
    const float capR = 0.56f * radius;

    {
        const auto shadowTip = tip.translated(radius * 0.24f, radius * 0.26f);
        g.setColour(juce::Colours::black.withAlpha(0.36f));
        g.fillEllipse(juce::Rectangle<float>(capR * 2.3f, capR * 1.9f).withCentre(shadowTip));
    }

    // Tronco de cone (espaço rotacionado): reflexo tipo "ambiente" céu/horizonte/chão
    // em vez de um degradê linear genérico de cromo.
    {
        g.saveState();
        const float leverAngle = std::atan2(tip.x - base.x, base.y - tip.y);
        g.addTransform(juce::AffineTransform::rotation(leverAngle).translated(base));

        const float length = base.getDistanceFrom(tip);
        const float wBase = 0.72f * radius;
        const float wTip = capR * 1.60f;

        juce::Path frustum;
        frustum.startNewSubPath(-wBase * 0.5f, socketR * 0.35f);
        frustum.lineTo(-wTip * 0.5f, -length);
        frustum.lineTo(wTip * 0.5f, -length);
        frustum.lineTo(wBase * 0.5f, socketR * 0.35f);
        frustum.closeSubPath();

        juce::ColourGradient chrome(juce::Colour(0xff34363a), -wTip * 0.5f, 0.0f,
                                    juce::Colour(0xff1c1e21), wTip * 0.5f, 0.0f, false);
        chrome.addColour(0.20f, juce::Colour(0xffcac5b8)); // faixa especular esquerda (níquel quente)
        chrome.addColour(0.38f, juce::Colour(0xff2c2e31)); // linha de horizonte
        chrome.addColour(0.60f, juce::Colour(0xff65686d));
        chrome.addColour(0.85f, juce::Colour(0xff9a968c)); // rebatida de luz na direita
        g.setGradientFill(chrome);
        g.fillPath(frustum);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(juce::Rectangle<float>(wBase * 1.15f, wBase * 0.42f)
                          .withCentre({ 0.0f, socketR * 0.25f }));
        g.restoreState();
    }

    // Corpo do cilindro atrás da tampa (crescente deslocado para o soquete)
    {
        const auto toBase = (base - tip) * (1.0f / juce::jmax(1.0f, base.getDistanceFrom(tip)));
        const auto crescentCentre = tip + toBase * (capR * 0.22f);
        const float sideR = capR * 1.08f;

        juce::ColourGradient sideGrad(juce::Colour(0xff8f8b81), crescentCentre.x - sideR * 0.6f, crescentCentre.y - sideR * 0.6f,
                                      juce::Colour(0xff1e2024), crescentCentre.x + sideR * 0.7f, crescentCentre.y + sideR * 0.7f, false);
        sideGrad.addColour(0.42f, juce::Colour(0xff2c2e31)); // horizonte
        sideGrad.addColour(0.5f, juce::Colour(0xff53585e));
        g.setGradientFill(sideGrad);
        g.fillEllipse(juce::Rectangle<float>(sideR * 2.0f, sideR * 2.0f).withCentre(crescentCentre));
    }

    // Tampa arredondada, vista quase de frente, em espaço de tela (brilho
    // sempre no topo-esquerda). Reflexo tipo ambiente + fresnel na borda.
    {
        const auto capRect = juce::Rectangle<float>(capR * 2.0f, capR * 2.0f).withCentre(tip);

        juce::ColourGradient capGrad(juce::Colour(0xffd8d3c5), tip.x, tip.y - capR * 0.9f,
                                     juce::Colour(0xff2a2d31), tip.x, tip.y + capR * 0.9f, false);
        capGrad.addColour(0.45f, juce::Colour(0xff35373b)); // horizonte
        capGrad.addColour(0.62f, juce::Colour(0xff787c82));
        capGrad.addColour(0.88f, juce::Colour(0xff4e5359));
        g.setGradientFill(capGrad);
        g.fillEllipse(capRect);

        // Fresnel: escurece a borda do domo independente da direção da luz.
        juce::ColourGradient fresnel(juce::Colours::transparentBlack, tip.x, tip.y,
                                     juce::Colours::black.withAlpha(0.32f), tip.x + capR, tip.y, true);
        fresnel.addColour(0.7f, juce::Colours::transparentBlack);
        g.setGradientFill(fresnel);
        g.fillEllipse(capRect);

        g.setColour(juce::Colours::black.withAlpha(0.18f));
        g.drawEllipse(capRect.reduced(capR * 0.22f), 0.8f);

        // Oclusão de contato do lado do soquete
        juce::Path contactOcclusion;
        const auto toBaseDir = (base - tip) * (1.0f / juce::jmax(1.0f, base.getDistanceFrom(tip)));
        const float contactAngle = std::atan2(toBaseDir.y, toBaseDir.x);
        contactOcclusion.addCentredArc(tip.x, tip.y, capR * 0.92f, capR * 0.92f, 0.0f,
                                       contactAngle - juce::MathConstants<float>::pi * 0.4f,
                                       contactAngle + juce::MathConstants<float>::pi * 0.4f, true);
        g.setColour(juce::Colours::black.withAlpha(0.28f));
        g.strokePath(contactOcclusion, juce::PathStrokeType(capR * 0.14f));

        // Fio de luz na borda iluminada
        juce::Path rimLight;
        rimLight.addCentredArc(tip.x, tip.y, capR * 0.94f, capR * 0.94f, 0.0f,
                               -juce::MathConstants<float>::pi * 0.85f,
                               -juce::MathConstants<float>::pi * 0.15f, true);
        g.setColour(juce::Colours::white.withAlpha(0.32f));
        g.strokePath(rimLight, juce::PathStrokeType(capR * 0.07f));

        // Catch-light alongado (não um blob radial simétrico)
        drawSpecularStreak(g, tip.translated(-capR * 0.24f, -capR * 0.30f), capR * 0.85f, capR * 0.30f,
                           lightAngle + juce::MathConstants<float>::halfPi, 0.6f);
        drawSpecularStreak(g, tip.translated(-capR * 0.20f, -capR * 0.24f), capR * 0.32f, capR * 0.14f,
                           lightAngle + juce::MathConstants<float>::halfPi, 0.5f);

        g.setColour(juce::Colours::black.withAlpha(0.38f));
        g.drawEllipse(capRect, 0.7f);
    }

    // ---- Envelhecimento: ruído + micro-riscos + pátina, clipado à arruela ----
    {
        static const juce::Image noise = GraphicsHelpers::createNoiseTexture(96, 96, 0x544f4747u);
        g.saveState();
        juce::Path clip;
        clip.addEllipse(washer);
        g.reduceClipRegion(clip);
        GraphicsHelpers::drawPanelTexture(g, noise, washer, 0.05f);
        GraphicsHelpers::drawScratches(g, washer, 0x42415254u, 9, 0.30f);
        juce::ColourGradient patina(juce::Colours::transparentBlack, centre.x, centre.y - radius,
                                    juce::Colour(0x30281f14), centre.x, centre.y + radius * 1.6f, false);
        g.setGradientFill(patina);
        g.fillEllipse(washer);
        g.restoreState();
    }

    if (highlighted || down)
    {
        g.setColour(juce::Colours::white.withAlpha(down ? 0.035f : 0.065f));
        g.fillEllipse(juce::Rectangle<float>(radius * 2.7f, radius * 2.7f).withCentre(centre));
    }
}
