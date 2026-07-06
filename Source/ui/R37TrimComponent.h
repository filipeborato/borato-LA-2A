#pragma once

#include <JuceHeader.h>

/*
    R37TrimComponent — o trimmer R37 (pre-emphasis do sidechain) como parafuso
    girável de verdade. Herda de juce::Slider para reaproveitar o arrasto
    rotativo e o SliderAttachment com o APVTS; o desenho usa o mesmo parafuso
    realista de RackScrew, mas o slot gira com o valor — ou seja, a única
    forma de alterar o valor resistivo é "girando a fenda" com o mouse,
    igual a ajustar um trimpot de verdade com uma chave de fenda.
*/
class R37TrimComponent final : public juce::Slider
{
public:
    R37TrimComponent();
    void paint(juce::Graphics&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R37TrimComponent)
};
