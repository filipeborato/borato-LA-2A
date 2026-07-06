#include "ToggleSwitchComponent.h"

/*
    Desenho baseado na ilustração vetorial clássica de "metal toggle switch"
    (Adobe Stock 103353637 / estilo OpenClipart): porca sextavada com cada
    faceta sombreada individualmente pela direção da luz, colar rosqueado,
    soquete escuro e alavanca bat-handle cromada com bola — incluindo o
    encurtamento de perspectiva quando a alavanca aponta "para baixo/fora".
*/

namespace
{
constexpr float lightAngle = -juce::MathConstants<float>::pi * 0.25f; // luz vinda do topo-esquerda

juce::Colour facetShade(float faceNormalAngle)
{
    // Brilho de cada faceta = quão alinhada a normal está com a luz.
    const float alignment = 0.5f + 0.5f * std::cos(faceNormalAngle - lightAngle);
    const float t = std::pow(alignment, 1.4f);
    return juce::Colour(0xff26282c).interpolatedWith(juce::Colour(0xfff2f4f7), t);
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
    // Escala com o componente (proporções do retângulo de design 200x260);
    // dimensionado para a porca não invadir os rótulos gravados do painel.
    const float radius = juce::jmin(bounds.getWidth() * 0.176f, bounds.getHeight() * 0.1355f);

    // ---- Sombra projetada do conjunto no painel ----
    {
        const auto shadowArea = juce::Rectangle<float>(radius * 3.1f, radius * 3.1f)
                                    .withCentre(centre.translated(radius * 0.16f, radius * 0.22f));
        juce::ColourGradient drop(juce::Colours::black.withAlpha(0.5f), shadowArea.getCentre().x, shadowArea.getCentre().y,
                                  juce::Colours::transparentBlack, shadowArea.getRight(), shadowArea.getCentre().y, true);
        g.setGradientFill(drop);
        g.fillEllipse(shadowArea);
    }

    // ---- Arruela circular usinada atrás da porca ----
    {
        const auto washer = juce::Rectangle<float>(radius * 3.16f, radius * 3.16f).withCentre(centre);
        juce::ColourGradient washGrad(juce::Colour(0xffe9ecef), centre.x - radius * 1.2f, centre.y - radius * 1.2f,
                                      juce::Colour(0xff3a3d42), centre.x + radius * 1.2f, centre.y + radius * 1.2f, false);
        washGrad.addColour(0.45f, juce::Colour(0xffb9bec4));
        washGrad.addColour(0.75f, juce::Colour(0xff70747a));
        g.setGradientFill(washGrad);
        g.fillEllipse(washer);

        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.drawEllipse(washer, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.35f));
        g.drawEllipse(washer.reduced(1.2f), 0.8f);
    }

    // ---- Porca sextavada: facetas chanfradas sombreadas uma a uma ----
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

    // Cada face do chanfro (trapézio entre o hexágono externo e o interno)
    // recebe seu próprio tom conforme a orientação para a luz.
    for (int i = 0; i < 6; ++i)
    {
        const int j = (i + 1) % 6;
        // Normal da face aponta no ângulo médio entre os dois vértices.
        const float faceNormal = juce::MathConstants<float>::twoPi * ((float) i + 0.5f) / 6.0f
                               + juce::MathConstants<float>::pi / 6.0f;

        juce::Path face;
        face.startNewSubPath(outerPts[(size_t) i]);
        face.lineTo(outerPts[(size_t) j]);
        face.lineTo(innerPts[(size_t) j]);
        face.lineTo(innerPts[(size_t) i]);
        face.closeSubPath();

        g.setColour(facetShade(faceNormal));
        g.fillPath(face);

        // Aresta entre facetas: claro no lado iluminado, escuro no oposto.
        g.setColour(juce::Colours::white.withAlpha(0.25f));
        g.drawLine({ outerPts[(size_t) i], innerPts[(size_t) i] }, 0.6f);
    }

    // Contorno externo da porca
    {
        juce::Path hexOutline;
        hexOutline.startNewSubPath(outerPts[0]);
        for (int i = 1; i < 6; ++i)
            hexOutline.lineTo(outerPts[(size_t) i]);
        hexOutline.closeSubPath();
        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.strokePath(hexOutline, juce::PathStrokeType(1.0f));
    }

    // Topo plano da porca (hexágono interno) — metal escovado claro
    {
        juce::Path topFace;
        topFace.startNewSubPath(innerPts[0]);
        for (int i = 1; i < 6; ++i)
            topFace.lineTo(innerPts[(size_t) i]);
        topFace.closeSubPath();

        juce::ColourGradient topGrad(juce::Colour(0xffe6e9ed), centre.x - hexInnerR, centre.y - hexInnerR,
                                     juce::Colour(0xff888d94), centre.x + hexInnerR, centre.y + hexInnerR, false);
        topGrad.addColour(0.55f, juce::Colour(0xffc4c9cf));
        g.setGradientFill(topGrad);
        g.fillPath(topFace);

        g.setColour(juce::Colours::black.withAlpha(0.30f));
        g.strokePath(topFace, juce::PathStrokeType(0.7f));
    }

    // ---- Colar rosqueado (anel elevado ao redor do soquete) ----
    const float collarR = radius * 0.88f;
    {
        const auto collar = juce::Rectangle<float>(collarR * 2.0f, collarR * 2.0f).withCentre(centre);
        juce::ColourGradient collarGrad(juce::Colour(0xfff2f4f6), centre.x - collarR * 0.7f, centre.y - collarR * 0.7f,
                                        juce::Colour(0xff4a4e54), centre.x + collarR * 0.7f, centre.y + collarR * 0.7f, false);
        collarGrad.addColour(0.5f, juce::Colour(0xffa9aeb5));
        g.setGradientFill(collarGrad);
        g.fillEllipse(collar);

        // Filetes de rosca visíveis no colar
        g.setColour(juce::Colours::black.withAlpha(0.22f));
        g.drawEllipse(collar.reduced(collarR * 0.14f), 0.7f);
        g.drawEllipse(collar.reduced(collarR * 0.28f), 0.7f);
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawEllipse(collar, 0.9f);
    }

    // ---- Soquete escuro de onde a alavanca emerge ----
    const float socketR = radius * 0.55f;
    {
        const auto socket = juce::Rectangle<float>(socketR * 2.0f, socketR * 2.0f).withCentre(centre);
        juce::ColourGradient socketGrad(juce::Colour(0xff020303), centre.x, centre.y - socketR * 0.4f,
                                        juce::Colour(0xff2e3136), centre.x, centre.y + socketR * 0.9f, true);
        g.setGradientFill(socketGrad);
        g.fillEllipse(socket);

        // Oclusão no topo interno + fio de luz na borda de baixo
        g.setColour(juce::Colours::black.withAlpha(0.85f));
        g.drawEllipse(socket, 1.4f);
        g.setColour(juce::Colours::white.withAlpha(0.28f));
        g.drawEllipse(socket.reduced(1.4f).translated(0.0f, 1.2f), 0.9f);
    }

    // ---- Alavanca bat-handle apontando "para fora da tela" ----
    // Perspectiva quase frontal: a alavanca sai do painel na direção do
    // observador, com inclinação bem discreta para cima/baixo. O que se vê é
    // a TAMPA arredondada da haste cilíndrica quase de frente (um círculo),
    // uma fresta do corpo do cilindro atrás dela (crescente) e o tronco de
    // cone na base — como na foto de referência da mini toggle switch.
    const float tilt = up ? -1.0f : 1.0f;
    const auto base = centre;
    const auto tip = centre.translated(-tilt * 0.05f * radius, tilt * 0.68f * radius);
    const float capR = 0.56f * radius; // raio da tampa da haste vista de frente

    // Sombra compacta projetada no painel/porca (luz do topo-esquerda)
    {
        const auto shadowTip = tip.translated(radius * 0.24f, radius * 0.26f);
        g.setColour(juce::Colours::black.withAlpha(0.38f));
        g.fillEllipse(juce::Rectangle<float>(capR * 2.3f, capR * 1.9f).withCentre(shadowTip));
    }

    // Tronco de cone na base (espaço rotacionado da alavanca)
    {
        g.saveState();
        const float leverAngle = std::atan2(tip.x - base.x, base.y - tip.y);
        g.addTransform(juce::AffineTransform::rotation(leverAngle).translated(base));

        const float length = base.getDistanceFrom(tip);
        const float wBase = 0.72f * radius;          // largura no soquete
        const float wTip = capR * 1.60f;             // abre em direção à tampa (perspectiva)

        juce::Path frustum;
        frustum.startNewSubPath(-wBase * 0.5f, socketR * 0.35f);
        frustum.lineTo(-wTip * 0.5f, -length);
        frustum.lineTo(wTip * 0.5f, -length);
        frustum.lineTo(wBase * 0.5f, socketR * 0.35f);
        frustum.closeSubPath();

        juce::ColourGradient chrome(juce::Colour(0xff2c2f33), -wTip * 0.5f, 0.0f,
                                    juce::Colour(0xff17191d), wTip * 0.5f, 0.0f, false);
        chrome.addColour(0.22f, juce::Colour(0xffeef1f4)); // faixa especular esquerda
        chrome.addColour(0.50f, juce::Colour(0xff70757c));
        chrome.addColour(0.82f, juce::Colour(0xffb9bdc3)); // rebatida de luz na direita
        g.setGradientFill(chrome);
        g.fillPath(frustum);

        // Anel de encaixe na base da haste (dentro do soquete)
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(juce::Rectangle<float>(wBase * 1.15f, wBase * 0.42f)
                          .withCentre({ 0.0f, socketR * 0.25f }));
        g.restoreState();
    }

    // Corpo do cilindro atrás da tampa: um crescente deslocado para o lado do
    // soquete, ligeiramente maior, em tom de cromo mais escuro.
    {
        const auto toBase = (base - tip) * (1.0f / juce::jmax(1.0f, base.getDistanceFrom(tip)));
        const auto crescentCentre = tip + toBase * (capR * 0.22f);
        const float sideR = capR * 1.08f;

        juce::ColourGradient sideGrad(juce::Colour(0xffb6bac0), crescentCentre.x - sideR * 0.6f, crescentCentre.y - sideR * 0.6f,
                                      juce::Colour(0xff1e2024), crescentCentre.x + sideR * 0.7f, crescentCentre.y + sideR * 0.7f, false);
        sideGrad.addColour(0.5f, juce::Colour(0xff53585e));
        g.setGradientFill(sideGrad);
        g.fillEllipse(juce::Rectangle<float>(sideR * 2.0f, sideR * 2.0f).withCentre(crescentCentre));
    }

    // Tampa arredondada da haste, vista quase de frente — desenhada em espaço
    // de tela para o brilho ficar sempre no topo-esquerda nos dois estados.
    {
        const auto capRect = juce::Rectangle<float>(capR * 2.0f, capR * 2.0f).withCentre(tip);
        juce::ColourGradient capGrad(juce::Colour(0xfff7f9fb), tip.x - capR * 0.30f, tip.y - capR * 0.35f,
                                     juce::Colour(0xff2a2d31), tip.x + capR * 0.85f, tip.y + capR * 0.85f, true);
        capGrad.addColour(0.30f, juce::Colour(0xffd4d8dc));
        capGrad.addColour(0.62f, juce::Colour(0xff8f959c));
        capGrad.addColour(0.88f, juce::Colour(0xff4e5359));
        g.setGradientFill(capGrad);
        g.fillEllipse(capRect);

        // Quina entre o domo da tampa e a parede do cilindro
        g.setColour(juce::Colours::black.withAlpha(0.20f));
        g.drawEllipse(capRect.reduced(capR * 0.22f), 0.8f);

        // Fio de luz na borda iluminada e sombra na borda oposta
        juce::Path rimLight;
        rimLight.addCentredArc(tip.x, tip.y, capR * 0.94f, capR * 0.94f, 0.0f,
                               -juce::MathConstants<float>::pi * 0.85f,
                               -juce::MathConstants<float>::pi * 0.15f, true);
        g.setColour(juce::Colours::white.withAlpha(0.55f));
        g.strokePath(rimLight, juce::PathStrokeType(capR * 0.09f));

        juce::Path rimDark;
        rimDark.addCentredArc(tip.x, tip.y, capR * 0.94f, capR * 0.94f, 0.0f,
                              juce::MathConstants<float>::pi * 0.15f,
                              juce::MathConstants<float>::pi * 0.85f, true);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.strokePath(rimDark, juce::PathStrokeType(capR * 0.09f));

        // Especular nítido no topo-esquerda
        juce::ColourGradient spec(juce::Colours::white.withAlpha(0.9f),
                                  tip.x - capR * 0.30f, tip.y - capR * 0.35f,
                                  juce::Colours::white.withAlpha(0.0f),
                                  tip.x + capR * 0.15f, tip.y + capR * 0.10f, true);
        g.setGradientFill(spec);
        g.fillEllipse(juce::Rectangle<float>(capR * 0.85f, capR * 0.7f)
                          .withCentre(tip.translated(-capR * 0.22f, -capR * 0.28f)));

        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.drawEllipse(capRect, 0.7f);
    }

    if (highlighted || down)
    {
        g.setColour(juce::Colours::white.withAlpha(down ? 0.035f : 0.065f));
        g.fillEllipse(juce::Rectangle<float>(radius * 2.7f, radius * 2.7f).withCentre(centre));
    }
}
