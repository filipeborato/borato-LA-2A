#pragma once

#include <JuceHeader.h>

/*
    SidechainPreEmphasis — modelo DSP do trimmer R37 do LA-2A.

    No hardware, o R37 ajusta quanto sinal chega ao amplificador de sidechain
    e, na prática, altera a sensibilidade de altas frequências da compressão.
    Aqui ele é modelado como um high-shelf aplicado SOMENTE ao caminho de
    sidechain (o áudio principal nunca passa por este filtro):

      - amount = 0.0  →  sidechain praticamente flat/full-band
      - amount = 1.0  →  agudos disparam mais compressão, graves comprimem menos

    Um corte broadband compensatório é embutido nos coeficientes para que subir
    o shelf não aumente a compressão global de forma explosiva.

    Controlado pelo parâmetro APVTS "r37PreEmphasis", exposto na GUI pelo
    R37TrimComponent (Source/ui/R37TrimComponent.h) — o parafuso do painel
    gira de verdade e mapeia o ângulo para este valor 0..1.
*/
class SidechainPreEmphasis
{
public:
    void prepare(double newSampleRate);
    void reset() noexcept;

    /// 0..1 — recalcula coeficientes apenas quando o valor muda (chamar por bloco).
    void setAmount(float newAmount0To1);

    float processSample(float x) noexcept;

private:
    void updateCoefficients();

    // ---- Calibração (ajustar de ouvido) ----
    static constexpr float shelfFreqHz       = 1800.0f; // centro da ênfase de HF (1k–3k)
    static constexpr float maxShelfDb        = 10.0f;   // boost do shelf com amount = 1 (+8..+12 dB)
    static constexpr float compensationRatio = 0.4f;    // corte broadband = -ratio * shelfDb

    double sampleRate = 44100.0;
    float amount = -1.0f; // -1 força o primeiro updateCoefficients()

    // Biquad TDF2 (RBJ high shelf), mono
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;
};
