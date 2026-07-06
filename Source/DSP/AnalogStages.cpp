#include "AnalogStages.h"

namespace
{
    float onePoleCoeff(float freqHz, double fs) noexcept
    {
        return 1.0f - std::exp((float) (-juce::MathConstants<double>::twoPi
                                        * (double) freqHz / fs));
    }
}

//==============================================================================
// AnalogInputStage
//==============================================================================
void AnalogInputStage::prepare(double sampleRate)
{
    const double fs = sampleRate > 0.0 ? sampleRate : 44100.0;
    hpCoeff = onePoleCoeff(highPassHz, fs);
    // Em sample rates baixos o LP de 20 kHz encosta em Nyquist — limitar
    lpCoeff = onePoleCoeff(juce::jmin(lowPassHz, (float) (fs * 0.45)), fs);
    reset();
}

void AnalogInputStage::reset() noexcept
{
    hpState.fill(0.0f);
    lpState.fill(0.0f);
}

float AnalogInputStage::processSample(float x, int channel, float amount) noexcept
{
    // HPF de 1 polo (x - lowpass) — rolloff de graves do transformer
    hpState[(size_t) channel] += hpCoeff * (x - hpState[(size_t) channel]);
    const float highPassed = x - hpState[(size_t) channel];

    // LPF suave de agudos
    lpState[(size_t) channel] += lpCoeff * (highPassed - lpState[(size_t) channel]);
    const float band = lpState[(size_t) channel];

    // Saturação leve, normalizada para ganho ~1 em sinal pequeno
    const float sat = std::tanh(band * satDrive) / satDrive;
    const float colored = band + satBlend * (sat - band);

    return x + amount * (colored - x);
}

//==============================================================================
// TubeOutputStage
//==============================================================================
void TubeOutputStage::prepare(double sampleRate)
{
    const double fs = sampleRate > 0.0 ? sampleRate : 44100.0;
    dcCoeff = onePoleCoeff(dcBlockHz, fs);
    reset();
}

void TubeOutputStage::reset() noexcept
{
    dcState.fill(0.0f);
}

float TubeOutputStage::tubeSaturate(float x, float driveAmount, float asym) noexcept
{
    // Assimetria via bias: gera predominância de 2ª harmônica
    const float biased = x * driveAmount + asym;
    const float y = std::tanh(biased);
    const float dc = std::tanh(asym);
    return ((y - dc) / juce::jmax(0.001f, 1.0f - std::abs(dc))) / driveAmount;
}

float TubeOutputStage::processSample(float x, int channel, float amount) noexcept
{
    const float sat = tubeSaturate(x, drive, asymBias * amount);
    const float colored = x + satBlend * amount * (sat - x);

    // DC blocker: a assimetria injeta um pequeno offset dependente de programa
    dcState[(size_t) channel] += dcCoeff * (colored - dcState[(size_t) channel]);
    return colored - dcState[(size_t) channel] * amount;
}

//==============================================================================
// TransformerOutputStage
//==============================================================================
void TransformerOutputStage::prepare(double sampleRate)
{
    const double fs = sampleRate > 0.0 ? sampleRate : 44100.0;
    lowCoeff = onePoleCoeff(lowRollHz, fs);
    reset();
}

void TransformerOutputStage::reset() noexcept
{
    lowState.fill(0.0f);
}

float TransformerOutputStage::processSample(float x, int channel, float amount) noexcept
{
    // Leve rolloff de subgraves (núcleo do trafo)
    lowState[(size_t) channel] += lowCoeff * (x - lowState[(size_t) channel]);
    const float rolled = x - lowRollMix * lowState[(size_t) channel];
    const float shaped = x + amount * (rolled - x);

    // Arredondamento suave de transientes
    const float sat = std::tanh(shaped * drive) / drive;
    return shaped + satBlend * amount * (sat - shaped);
}
