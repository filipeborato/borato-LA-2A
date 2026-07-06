#include "OptoCompressor.h"

namespace
{
    float onePoleCoeff(float ms, double fs) noexcept
    {
        if (ms <= 0.0f)
            return 1.0f;
        return 1.0f - std::exp((float) (-1.0 / ((double) ms * 0.001 * fs)));
    }

    // dB → linear sem chamada a pow(10, x): exp(x * ln(10)/20)
    float dbToGainFast(float db) noexcept
    {
        return std::exp(db * 0.11512925f);
    }
}

void OptoCompressor::prepare(double newSampleRate, int, int newNumChannels)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    numChannels = juce::jlimit(1, 2, newNumChannels);

    t4.prepare(sampleRate);
    r37FilterL.prepare(sampleRate);
    r37FilterR.prepare(sampleRate);
    inputStage.prepare(sampleRate);
    tubeStage.prepare(sampleRate);
    outputStage.prepare(sampleRate);

    detectorCoeff = onePoleCoeff(detectorMs, sampleRate);
    grMeterCoeff  = onePoleCoeff(grMeterMs, sampleRate);
    vuCoeff       = onePoleCoeff(vuMeterMs, sampleRate);

    for (auto* smoothed : { &inputGain, &makeupGain, &outputGain,
                            &mixAmount, &analogAmount, &sidechainDrive })
        smoothed->reset(sampleRate, parameterSmoothingS);

    inputGain.setCurrentAndTargetValue(1.0f);
    makeupGain.setCurrentAndTargetValue(1.0f);
    outputGain.setCurrentAndTargetValue(1.0f);
    mixAmount.setCurrentAndTargetValue(1.0f);
    analogAmount.setCurrentAndTargetValue(0.5f);
    sidechainDrive.setCurrentAndTargetValue(1.0f);

    reset();
}

void OptoCompressor::reset()
{
    t4.reset();
    r37FilterL.reset();
    r37FilterR.reset();
    inputStage.reset();
    tubeStage.reset();
    outputStage.reset();

    detector = 0.0f;
    feedbackL = 0.0f;
    feedbackR = 0.0f;
    grMeterDb = 0.0f;
    vuEnvelope = 0.0f;
}

void OptoCompressor::setInputTrimDb(float db) noexcept
{
    inputGain.setTargetValue(juce::Decibels::decibelsToGain(db));
}

void OptoCompressor::setPeakReduction(float value0To100) noexcept
{
    // Mapeamento musical: quadrático no knob, depois em dB. PR controla apenas
    // o drive do sidechain — nunca o nível do caminho de áudio.
    const float prNorm = juce::jlimit(0.0f, 1.0f, value0To100 / 100.0f);
    const float driveDb = juce::jmap(prNorm * prNorm, sidechainDriveMinDb, sidechainDriveMaxDb);
    sidechainDrive.setTargetValue(juce::Decibels::decibelsToGain(driveDb));
}

void OptoCompressor::setMakeupGainDb(float db) noexcept
{
    makeupGain.setTargetValue(juce::Decibels::decibelsToGain(db));
}

void OptoCompressor::setR37PreEmphasis(float value0To1) noexcept
{
    r37Amount = juce::jlimit(0.0f, 1.0f, value0To1);
}

void OptoCompressor::setAnalogAmount(float value0To1) noexcept
{
    analogAmount.setTargetValue(juce::jlimit(0.0f, 1.0f, value0To1));
}

void OptoCompressor::setMix(float value0To1) noexcept
{
    mixAmount.setTargetValue(juce::jlimit(0.0f, 1.0f, value0To1));
}

void OptoCompressor::setOutputTrimDb(float db) noexcept
{
    outputGain.setTargetValue(juce::Decibels::decibelsToGain(db));
}

void OptoCompressor::process(juce::AudioBuffer<float>& buffer) noexcept
{
    const int numSamples = buffer.getNumSamples();
    const int channels = juce::jmin(buffer.getNumChannels(), numChannels);
    if (numSamples == 0 || channels == 0)
        return;

    // Atualizações por bloco (fora do loop de samples)
    r37FilterL.setAmount(r37Amount);
    r37FilterR.setAmount(r37Amount);
    t4.updateAdaptiveRelease();

    auto* left = buffer.getWritePointer(0);
    auto* right = channels > 1 ? buffer.getWritePointer(1) : nullptr;
    const bool stereo = right != nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float analog = analogAmount.getNextValue();
        const float trim = inputGain.getNextValue();

        // 1-3. Input trim + coloração do transformer/line amp de entrada
        const float dryL = left[i] * trim;
        const float dryR = stereo ? right[i] * trim : dryL;
        float xL = inputStage.processSample(dryL, 0, analog);
        float xR = stereo ? inputStage.processSample(dryR, 1, analog) : xL;

        // 4. Sidechain feedback: sinal comprimido do sample anterior
        //    → R37 pre-emphasis (audio rate, por canal) → linked rectifier
        const float scL = r37FilterL.processSample(feedbackL);
        const float scR = r37FilterR.processSample(feedbackR);
        const float rectified = juce::jmax(std::abs(scL), std::abs(scR));
        detector += detectorCoeff * (rectified - detector);

        // 5. Peak Reduction drive + não-linearidade do painel EL
        const float scValue = detector * sidechainDrive.getNextValue();
        const float overdrive = juce::jmax(0.0f, scValue - elThresholdLin);
        float targetLight = 1.0f - std::exp(-elPanelDrive * overdrive);
        targetLight = std::pow(juce::jmax(0.0f, targetLight), elGamma);

        // 6-7. Memória do T4 e curva light → GR
        const float light = t4.processSample(targetLight);
        const float grDb = t4.lightToGainReductionDb(light, limitMode);
        t4.noteGainReductionDb(grDb);
        const float grLinear = dbToGainFast(grDb);

        // 8. Atenuação óptica no caminho de áudio
        float wetL = xL * grLinear;
        float wetR = xR * grLinear;

        // 11. Feedback tap: saída comprimida PRÉ-makeup (evita loop algébrico
        //     e evita que o Gain knob module a compressão)
        feedbackL = wetL;
        feedbackR = wetR;

        // 9-10. Makeup gain → tube amp → output transformer
        const float makeup = makeupGain.getNextValue();
        wetL = outputStage.processSample(tubeStage.processSample(wetL * makeup, 0, analog), 0, analog);
        wetR = stereo
            ? outputStage.processSample(tubeStage.processSample(wetR * makeup, 1, analog), 1, analog)
            : wetL;

        // 12. Mix paralelo + output trim
        const float mix = mixAmount.getNextValue();
        const float outGain = outputGain.getNextValue();
        const float outL = (dryL + mix * (wetL - dryL)) * outGain;
        const float outR = (dryR + mix * (wetR - dryR)) * outGain;

        left[i] = outL;
        if (stereo)
            right[i] = outR;

        // Medição: GR suavizada + envelope de nível de saída
        grMeterDb += grMeterCoeff * (grDb - grMeterDb);
        const float outAbs = juce::jmax(std::abs(outL), std::abs(outR));
        vuEnvelope += vuCoeff * (outAbs - vuEnvelope);
    }

    sanitizeState();
}

void OptoCompressor::sanitizeState() noexcept
{
    // Guarda contra NaN/Inf se propagando pelo loop de feedback. Checagem
    // barata uma vez por bloco; em caso de corrupção, zera os estados.
    if (! std::isfinite(feedbackL) || ! std::isfinite(feedbackR)
        || ! std::isfinite(detector) || ! std::isfinite(grMeterDb))
    {
        reset();
    }
}
