#pragma once

#include <JuceHeader.h>
#include <array>

/*
    AnalogStages — coloração analógica leve e estável do caminho de áudio.

    Três estágios independentes, todos com intensidade controlada pelo
    parâmetro "analog" (0..1) via crossfade dry/colorido:

      - AnalogInputStage:      transformer de entrada / line amp
                               (HPF suave ~25 Hz, LPF suave ~20 kHz,
                                saturação tanh leve dependente de nível)
      - TubeOutputStage:       amplificador valvulado de saída
                               (saturação assimétrica com bias → 2ª harmônica,
                                DC blocker depois)
      - TransformerOutputStage: transformer de saída
                               (saturação suave, arredondamento de transientes,
                                leve aperto de subgraves)

    Sem oversampling: os drives são conservadores de propósito para manter o
    aliasing desprezível e não adicionar latência. Estados por canal em arrays
    fixos (máx. 2 canais) — nenhuma alocação no processamento.
*/

class AnalogInputStage
{
public:
    void prepare(double sampleRate);
    void reset() noexcept;

    /// amount 0..1: intensidade da coloração. amount = 0 é bit-transparente.
    float processSample(float x, int channel, float amount) noexcept;

private:
    // ---- Calibração ----
    static constexpr float highPassHz = 25.0f;    // rolloff de graves do trafo de entrada
    static constexpr float lowPassHz  = 20000.0f; // rolloff suave de agudos
    static constexpr float satDrive   = 1.3f;     // drive da tanh (leve)
    static constexpr float satBlend   = 0.5f;     // quanto da saturação entra com amount = 1

    float hpCoeff = 0.0f, lpCoeff = 1.0f;
    std::array<float, 2> hpState {};
    std::array<float, 2> lpState {};
};

class TubeOutputStage
{
public:
    void prepare(double sampleRate);
    void reset() noexcept;

    float processSample(float x, int channel, float amount) noexcept;

private:
    // ---- Calibração ----
    static constexpr float drive     = 1.2f;   // ganho interno da válvula
    static constexpr float asymBias  = 0.08f;  // bias → assimetria → 2ª harmônica
    static constexpr float satBlend  = 0.6f;   // mistura máxima da saturação
    static constexpr float dcBlockHz = 10.0f;  // DC blocker pós-assimetria

    static float tubeSaturate(float x, float driveAmount, float asym) noexcept;

    float dcCoeff = 0.0f;
    std::array<float, 2> dcState {};
};

class TransformerOutputStage
{
public:
    void prepare(double sampleRate);
    void reset() noexcept;

    float processSample(float x, int channel, float amount) noexcept;

private:
    // ---- Calibração ----
    static constexpr float drive       = 1.1f;  // saturação bem suave
    static constexpr float satBlend    = 0.45f; // arredondamento de transientes
    static constexpr float lowRollHz   = 15.0f; // leve perda de subgraves do trafo
    static constexpr float lowRollMix  = 0.5f;  // quanto do rolloff entra com amount = 1

    float lowCoeff = 0.0f;
    std::array<float, 2> lowState {};
};
