#pragma once

#include <JuceHeader.h>
#include "DSP/OptoCompressor.h"

namespace Params
{
inline constexpr auto inputTrim = "inputTrim";
inline constexpr auto gain = "gain";
inline constexpr auto peakReduction = "peakReduction";
inline constexpr auto meterMode = "meterMode";
inline constexpr auto mode = "mode";
inline constexpr auto power = "power";
// R37: parafuso girável (R37TrimComponent) que controla o filtro de
// pre-emphasis do sidechain; salvo/restaurado no estado e automatizável.
inline constexpr auto r37PreEmphasis = "r37PreEmphasis";
inline constexpr auto analog = "analog";
inline constexpr auto mix = "mix";
inline constexpr auto output = "output";
}

class BoratoLA2AAudioProcessor final : public juce::AudioProcessor
{
public:
    BoratoLA2AAudioProcessor();
    ~BoratoLA2AAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void reset() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Borato LA-2A"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    /// Nível de saída suavizado em dBFS para o VU (modos +10/+4).
    float getMeterDb() const noexcept { return meterDb.load(std::memory_order_relaxed); }

    /// Gain reduction POSITIVA para display: 7 dB de redução → retorna 7.0f.
    /// (A GUI nega o valor para posicionar a agulha do modo GR.)
    float getGainReductionDb() const noexcept { return gainReductionDb.load(std::memory_order_relaxed); }

private:
    void pushParametersToCompressor();

    std::atomic<float>* inputTrimParam = nullptr;
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* peakParam = nullptr;
    std::atomic<float>* modeParam = nullptr;
    std::atomic<float>* powerParam = nullptr;
    std::atomic<float>* r37Param = nullptr;
    std::atomic<float>* analogParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* outputParam = nullptr;

    OptoCompressor optoCompressor;

    std::atomic<float> meterDb { -60.0f };
    std::atomic<float> gainReductionDb { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoratoLA2AAudioProcessor)
};
