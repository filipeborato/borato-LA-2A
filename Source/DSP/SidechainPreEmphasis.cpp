#include "SidechainPreEmphasis.h"

void SidechainPreEmphasis::prepare(double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    amount = -1.0f; // força recomputar na próxima setAmount()
    setAmount(0.35f);
    reset();
}

void SidechainPreEmphasis::reset() noexcept
{
    z1 = 0.0f;
    z2 = 0.0f;
}

void SidechainPreEmphasis::setAmount(float newAmount0To1)
{
    const float clamped = juce::jlimit(0.0f, 1.0f, newAmount0To1);
    if (std::abs(clamped - amount) < 1.0e-4f)
        return;

    amount = clamped;
    updateCoefficients();
}

void SidechainPreEmphasis::updateCoefficients()
{
    const float shelfDb = amount * maxShelfDb;

    // High shelf RBJ (S = 1)
    const double A     = std::pow(10.0, (double) shelfDb / 40.0);
    const double w0    = juce::MathConstants<double>::twoPi * (double) shelfFreqHz / sampleRate;
    const double cosW0 = std::cos(w0);
    const double sinW0 = std::sin(w0);
    const double alpha = 0.5 * sinW0 * std::sqrt(2.0); // S = 1
    const double sqrtA = std::sqrt(A);

    const double a0 = (A + 1.0) - (A - 1.0) * cosW0 + 2.0 * sqrtA * alpha;
    const double norm = 1.0 / a0;

    // Corte broadband compensatório para não explodir a compressão global
    const double comp = std::pow(10.0, (double) (-compensationRatio * shelfDb) / 20.0);

    b0 = (float) (comp * norm * A * ((A + 1.0) + (A - 1.0) * cosW0 + 2.0 * sqrtA * alpha));
    b1 = (float) (comp * norm * -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW0));
    b2 = (float) (comp * norm * A * ((A + 1.0) + (A - 1.0) * cosW0 - 2.0 * sqrtA * alpha));
    a1 = (float) (norm * 2.0 * ((A - 1.0) - (A + 1.0) * cosW0));
    a2 = (float) (norm * ((A + 1.0) - (A - 1.0) * cosW0 - 2.0 * sqrtA * alpha));
}

float SidechainPreEmphasis::processSample(float x) noexcept
{
    // Transposed Direct Form II
    const float y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    return y;
}
