#pragma once

#include <JuceHeader.h>
#include "T4OpticalCellModel.h"
#include "SidechainPreEmphasis.h"
#include "AnalogStages.h"

/*
    OptoCompressor — compressor óptico feedback estilo LA-2A.

    Fluxo de sinal:

        Input → Input Trim → Input Transformer/Line Amp
              → [Atenuação T4]  ← sidechain feedback
              → Makeup Gain → Tube Amp → Output Transformer
              → Mix (paralelo) → Output Trim

    Sidechain (topologia FEEDBACK — olha para a saída já comprimida do sample
    anterior, pré-makeup, evitando loop algébrico e evitando que o Gain knob
    realimente a compressão):

        feedback tap → filtro R37 pre-emphasis (por canal, a audio rate)
                     → retificação linked (máx |L|,|R|) → smoothing
                     → Peak Reduction drive → não-linearidade do painel EL
                     → T4 (células rápida/lenta + memória)
                     → curva light → gainReductionDb

    Peak Reduction NÃO é ganho de entrada do áudio: só escala o drive do
    sidechain. Gain é makeup/output depois da redução óptica.

    Realtime-safe: nenhuma alocação/lock/log no process(); estados por canal
    em arrays fixos; coeficientes atualizados por bloco.
*/
class OptoCompressor
{
public:
    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    // Setters chamados por bloco a partir do APVTS (thread de áudio, sem locks)
    void setInputTrimDb(float db) noexcept;
    void setPeakReduction(float value0To100) noexcept;
    void setMakeupGainDb(float db) noexcept;
    void setLimitMode(bool shouldLimit) noexcept   { limitMode = shouldLimit; }
    void setR37PreEmphasis(float value0To1) noexcept;
    void setAnalogAmount(float value0To1) noexcept;
    void setMix(float value0To1) noexcept;
    void setOutputTrimDb(float db) noexcept;

    void process(juce::AudioBuffer<float>& buffer) noexcept;

    /// GR suavizada em dB, negativa quando reduzindo (ex.: -7.0f = 7 dB de GR).
    float getGainReductionDb() const noexcept { return grMeterDb; }

    /// GR positiva para display (7 dB de redução → retorna 7.0f).
    float getGainReductionForMeter() const noexcept { return -grMeterDb; }

    /// Nível de saída suavizado (balística lenta tipo VU), em dBFS.
    float getVuLevelDb() const noexcept
    {
        return juce::Decibels::gainToDecibels(vuEnvelope, -60.0f);
    }

private:
    void sanitizeState() noexcept;

    // ---- Calibração (ajustar de ouvido) ----
    static constexpr float sidechainDriveMinDb = -18.0f; // PR = 0 (sem compressão)
    static constexpr float sidechainDriveMaxDb = 40.0f;  // PR = 100
    static constexpr float elThresholdLin      = 0.02f;  // piso do painel EL
    static constexpr float elPanelDrive        = 0.9f;   // inclinação da curva do EL
    static constexpr float elGamma             = 0.8f;   // suaviza o joelho da luz
    static constexpr float detectorMs          = 2.0f;   // anti-click do retificador
    static constexpr float grMeterMs           = 25.0f;  // suavização da GR para o VU
    static constexpr float vuMeterMs           = 200.0f; // envelope do nível de saída
    static constexpr float parameterSmoothingS = 0.03f;  // smoothing dos knobs

    T4OpticalCellModel t4;
    SidechainPreEmphasis r37FilterL, r37FilterR;
    AnalogInputStage inputStage;
    TubeOutputStage tubeStage;
    TransformerOutputStage outputStage;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> makeupGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixAmount;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> analogAmount;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sidechainDrive;

    bool limitMode = false;
    float r37Amount = 0.35f;

    // Estado do sidechain / feedback
    float detector = 0.0f;
    float detectorCoeff = 0.0f;
    float feedbackL = 0.0f; // saída comprimida (pré-makeup) do sample anterior
    float feedbackR = 0.0f;

    // Medição
    float grMeterDb = 0.0f;
    float grMeterCoeff = 0.0f;
    float vuEnvelope = 0.0f;
    float vuCoeff = 0.0f;

    double sampleRate = 44100.0;
    int numChannels = 2;
};
