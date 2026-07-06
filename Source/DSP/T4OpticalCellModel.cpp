#include "T4OpticalCellModel.h"

float T4OpticalCellModel::msToCoeff(float ms, double fs) noexcept
{
    if (ms <= 0.0f)
        return 1.0f;
    return 1.0f - std::exp((float) (-1.0 / ((double) ms * 0.001 * fs)));
}

void T4OpticalCellModel::prepare(double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;

    attackCoeff      = msToCoeff(attackMs, sampleRate);
    fastReleaseCoeff = msToCoeff(fastReleaseMs, sampleRate);
    slowAttackCoeff  = msToCoeff(slowAttackMs, sampleRate);
    memoryRiseCoeff  = msToCoeff(memoryRiseMs, sampleRate);
    memoryFallCoeff  = msToCoeff(memoryFallMs, sampleRate);

    reset();
    updateAdaptiveRelease();
}

void T4OpticalCellModel::reset() noexcept
{
    fastCell = 0.0f;
    slowCell = 0.0f;
    memory   = 0.0f;
    lastGainReductionDb = 0.0f;
}

void T4OpticalCellModel::updateAdaptiveRelease() noexcept
{
    // Profundidade recente: combina a GR instantânea com a memória de longo
    // prazo. Quanto mais compressão sustentada, mais lenta a recuperação.
    const float instantDepth = juce::jlimit(0.0f, 1.0f,
        std::abs(lastGainReductionDb) / depthFullScaleDb);
    const float depth = juce::jlimit(0.0f, 1.0f,
        0.45f * instantDepth + 0.55f * memory);

    const float adaptiveSlowReleaseMs = juce::jmap(depth, slowReleaseMinMs, slowReleaseMaxMs);
    slowReleaseCoeff = msToCoeff(adaptiveSlowReleaseMs, sampleRate);
}

float T4OpticalCellModel::processSample(float targetLight) noexcept
{
    targetLight = juce::jlimit(0.0f, 1.0f, targetLight);

    // Célula rápida: ataque ~10 ms, primeiro estágio de release ~60 ms
    if (targetLight > fastCell)
        fastCell += attackCoeff * (targetLight - fastCell);
    else
        fastCell += fastReleaseCoeff * (targetLight - fastCell);

    // Célula lenta: cauda program-dependent (0.5 s a 5 s)
    if (targetLight > slowCell)
        slowCell += slowAttackCoeff * (targetLight - slowCell);
    else
        slowCell += slowReleaseCoeff * (targetLight - slowCell);

    // Memória de longo prazo: "aquecimento" que atrasa o release futuro
    if (targetLight > memory)
        memory += memoryRiseCoeff * (targetLight - memory);
    else
        memory += memoryFallCoeff * (targetLight - memory);

    return fastWeight * fastCell + (1.0f - fastWeight) * slowCell;
}

float T4OpticalCellModel::lightToGainReductionDb(float light, bool limitMode) const noexcept
{
    light = juce::jlimit(0.0f, 1.0f, light);

    if (! limitMode)
    {
        // Compress: knee suave, menos profundidade para a mesma luz
        return -compressMaxGrDb * (1.0f - std::exp(-compressCurveK * light));
    }

    // Limit: curva mais íngreme, mais redução para o mesmo Peak Reduction
    return -limitMaxGrDb * (1.0f - std::exp(-limitCurveK * light));
}
