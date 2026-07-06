#include "R37TrimComponent.h"
#include "RackScrew.h"

R37TrimComponent::R37TrimComponent()
{
    setOpaque(false);
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    // Mesmo curso angular dos knobs do painel (270 graus), para o ato de
    // "girar a fenda" ser consistente com Gain/Peak Reduction.
    setRotaryParameters(juce::degreesToRadians(225.0f), juce::degreesToRadians(495.0f), true);
    // Sensibilidade mais alta (curso mais "duro"): um trimmer de precisão
    // não deve disparar de ponta a ponta com um arrasto curto de mouse.
    setMouseDragSensitivity(320);
}

void R37TrimComponent::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced(8.0f);
    const auto centre = bounds.getCentre();
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // O slot do parafuso É o indicador: gira proporcionalmente ao valor,
    // exatamente como girar fisicamente o trimmer altera o valor resistivo.
    const auto normalised = (float) valueToProportionOfLength(getValue());
    const auto rotaryParams = getRotaryParameters();
    const float slotAngle = juce::jmap(normalised, rotaryParams.startAngleRadians, rotaryParams.endAngleRadians);

    RackScrew::draw(g, centre, radius, slotAngle);
}
