#include "PluginProcessor.h"
#include "PluginEditor.h"

BoratoLA2AAudioProcessor::BoratoLA2AAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    inputTrimParam = apvts.getRawParameterValue(Params::inputTrim);
    gainParam = apvts.getRawParameterValue(Params::gain);
    peakParam = apvts.getRawParameterValue(Params::peakReduction);
    modeParam = apvts.getRawParameterValue(Params::mode);
    powerParam = apvts.getRawParameterValue(Params::power);
    r37Param = apvts.getRawParameterValue(Params::r37PreEmphasis);
    analogParam = apvts.getRawParameterValue(Params::analog);
    mixParam = apvts.getRawParameterValue(Params::mix);
    outputParam = apvts.getRawParameterValue(Params::output);
}

juce::AudioProcessorValueTreeState::ParameterLayout BoratoLA2AAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { Params::inputTrim, 1 },
        "Input", juce::NormalisableRange<float> { -24.0f, 24.0f, 0.1f }, 0.0f));

    // Peak Reduction: drive do sidechain (não é ganho de entrada do áudio)
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { Params::peakReduction, 1 },
        "Peak Reduction", juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 35.0f));

    // Gain: makeup/output gain DEPOIS da redução óptica
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { Params::gain, 1 },
        "Gain", juce::NormalisableRange<float> { -10.0f, 40.0f, 0.1f }, 0.0f));

    // Mode: Compress (curva suave) / Limit (mais agressiva, mais GR).
    // Com 2 choices o ButtonAttachment do toggle da GUI segue funcionando.
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { Params::mode, 1 },
        "Mode", juce::StringArray { "Compress", "Limit" }, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { Params::meterMode, 1 },
        "Meter", juce::StringArray { "+10", "+4", "GR" }, 2));

    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { Params::power, 1 },
        "Power", true));

    // R37: extensão DSP do parafuso (decorativo na GUI atual). Já afeta o
    // sidechain e é automatizável; futuro: mini trimpot/painel advanced.
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { Params::r37PreEmphasis, 1 },
        "R37 / HF Sensitivity", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.35f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { Params::analog, 1 },
        "Analog", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { Params::mix, 1 },
        "Mix", juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { Params::output, 1 },
        "Output", juce::NormalisableRange<float> { -24.0f, 24.0f, 0.1f }, 0.0f));

    return layout;
}

void BoratoLA2AAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    optoCompressor.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    // Empurra os valores atuais dos knobs e faz snap: sem isso o primeiro
    // bloco faz ramp de 30 ms partindo dos defaults (burst de até 24 dB).
    pushParametersToCompressor();
    optoCompressor.snapParameters();
}

void BoratoLA2AAudioProcessor::reset()
{
    optoCompressor.reset();
}

void BoratoLA2AAudioProcessor::pushParametersToCompressor()
{
    optoCompressor.setInputTrimDb(inputTrimParam->load(std::memory_order_relaxed));
    optoCompressor.setPeakReduction(peakParam->load(std::memory_order_relaxed));
    optoCompressor.setMakeupGainDb(gainParam->load(std::memory_order_relaxed));
    optoCompressor.setLimitMode(modeParam->load(std::memory_order_relaxed) >= 0.5f);
    optoCompressor.setPowerOn(powerParam->load(std::memory_order_relaxed) >= 0.5f);
    optoCompressor.setR37PreEmphasis(r37Param->load(std::memory_order_relaxed));
    optoCompressor.setAnalogAmount(analogParam->load(std::memory_order_relaxed));
    optoCompressor.setMix(mixParam->load(std::memory_order_relaxed) * 0.01f);
    optoCompressor.setOutputTrimDb(outputParam->load(std::memory_order_relaxed));
}

void BoratoLA2AAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();
    const auto inputChannels = getTotalNumInputChannels();
    const auto outputChannels = getTotalNumOutputChannels();
    for (int channel = inputChannels; channel < outputChannels; ++channel)
        buffer.clear(channel, 0, numSamples);

    // Power off vira crossfade de ~20 ms dentro do compressor (sem clique);
    // o VU segue mostrando o nível que passa e a GR decai a zero.
    pushParametersToCompressor();

    optoCompressor.process(buffer);

    meterDb.store(optoCompressor.getVuLevelDb(), std::memory_order_relaxed);
    gainReductionDb.store(optoCompressor.getGainReductionForMeter(), std::memory_order_relaxed);
}

bool BoratoLA2AAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo())
        && output == layouts.getMainInputChannelSet();
}

void BoratoLA2AAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void BoratoLA2AAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* BoratoLA2AAudioProcessor::createEditor()
{
    return new BoratoLA2AAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BoratoLA2AAudioProcessor();
}
