#pragma once

#include <JuceHeader.h>

/*
    T4OpticalCellModel — modelo da célula óptica T4B do LA-2A.

    O T4 real é um painel eletroluminescente iluminando fotorresistores CdS.
    O atenuador resultante equivale a um divisor resistivo:

        g_T4 = R_LDR / (R_series + R_LDR)

    Em DSP o modelo é organizado como: luz alvo (vinda do detector + EL panel,
    calculada pelo OptoCompressor) → memória da célula (duas células com
    constantes de tempo diferentes + memória de longo prazo) → curva
    light → gainReductionDb.

    O comportamento característico é o release em dois estágios e dependente
    de programa: recuperação inicial rápida (~60 ms) seguida de uma cauda
    lenta (0.5 s a 5 s) que fica mais lenta quanto mais compressão sustentada
    houve antes.
*/
class T4OpticalCellModel
{
public:
    void prepare(double newSampleRate);
    void reset() noexcept;

    /// Recalcula o release lento adaptativo a partir da profundidade recente
    /// de compressão. Chamar uma vez por bloco (usa exp; barato fora do loop).
    void updateAdaptiveRelease() noexcept;

    /// Avança as células um sample em direção à luz alvo e devolve a luz efetiva.
    float processSample(float targetLight) noexcept;

    /// Curva luz → redução de ganho em dB (valor negativo quando reduz).
    float lightToGainReductionDb(float light, bool limitMode) const noexcept;

    /// O compressor informa a GR aplicada, alimentando a adaptação do release.
    void noteGainReductionDb(float grDb) noexcept { lastGainReductionDb = grDb; }

    float getLastGainReductionDb() const noexcept { return lastGainReductionDb; }

private:
    static float msToCoeff(float ms, double fs) noexcept;

    // ---- Calibração (ajustar de ouvido) ----
    static constexpr float attackMs         = 10.0f;   // ataque da célula rápida
    static constexpr float fastReleaseMs    = 60.0f;   // 1º estágio de recuperação
    static constexpr float slowAttackMs     = 80.0f;   // quão rápido a cauda "carrega"
    static constexpr float slowReleaseMinMs = 500.0f;  // cauda mínima (pouca compressão)
    static constexpr float slowReleaseMaxMs = 5000.0f; // cauda máxima (compressão profunda sustentada)
    static constexpr float memoryRiseMs     = 400.0f;  // memória de longo prazo (sobe)
    static constexpr float memoryFallMs     = 8000.0f; // memória de longo prazo (esfria)
    static constexpr float fastWeight       = 0.72f;   // mistura fastCell/slowCell
    static constexpr float depthFullScaleDb = 20.0f;   // GR que conta como "profundidade total"

    static constexpr float compressCurveK  = 2.2f;  // knee suave, curva gradual
    static constexpr float compressMaxGrDb = 24.0f;
    static constexpr float limitCurveK     = 3.8f;  // knee firme, resposta travada
    static constexpr float limitMaxGrDb    = 32.0f;

    // ---- Estado ----
    float fastCell = 0.0f;
    float slowCell = 0.0f;
    float memory   = 0.0f; // "aquecimento" de longo prazo do painel EL/célula
    float lastGainReductionDb = 0.0f;

    float attackCoeff      = 0.0f;
    float fastReleaseCoeff = 0.0f;
    float slowAttackCoeff  = 0.0f;
    float slowReleaseCoeff = 0.0f;
    float memoryRiseCoeff  = 0.0f;
    float memoryFallCoeff  = 0.0f;

    double sampleRate = 44100.0;
};
