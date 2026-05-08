#include "PluginProcessor.h"
#include "PluginEditor.h"

CromaSatAudioProcessor::CromaSatAudioProcessor()
     : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "Parameters", createParameterLayout()),
       forwardFFT (std::make_unique<juce::dsp::FFT>(fftOrder)),
       window (std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::hann))
{
    for (auto& f : filters)
        f = std::make_unique<juce::dsp::LinkwitzRileyFilter<float>>();
}

CromaSatAudioProcessor::~CromaSatAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout CromaSatAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "inputGain", 1 }, "Input Gain", -24.0f, 24.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "globalMix", 1 }, "Global Mix", 0.0f, 100.0f, 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "outputGain", 1 }, "Output Gain", -24.0f, 24.0f, 0.0f));

    // Crossover Frequencies
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "crossFreq1", 1 }, "Crossover 1", 20.0f, 200.0f, 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "crossFreq2", 1 }, "Crossover 2", 200.0f, 1000.0f, 500.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "crossFreq3", 1 }, "Crossover 3", 1000.0f, 5000.0f, 2000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "crossFreq4", 1 }, "Crossover 4", 5000.0f, 10000.0f, 7000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "crossFreq5", 1 }, "Crossover 5", 10000.0f, 20000.0f, 15000.0f));

    // Per-band Parameters
    for (int i = 0; i < numBands; ++i)
    {
        juce::String id = "band" + juce::String(i);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id + "Drive", 1 }, "Drive " + juce::String(i+1), 0.0f, 100.0f, 20.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id + "Mix", 1 }, "Mix " + juce::String(i+1), 0.0f, 100.0f, 100.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id + "Level", 1 }, "Level " + juce::String(i+1), -12.0f, 12.0f, 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id + "Type", 1 }, "Type " + juce::String(i+1), 
            juce::StringArray { "Tube", "Tape", "Transformer", "Solid State", "Distortion", "Crush" }, 0));
        params.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id + "Enabled", 1 }, "Enabled " + juce::String(i+1), true));
    }

    return { params.begin(), params.end() };
}

void CromaSatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    for (int i = 0; i < (numBands - 1); ++i)
    {
        // Lowpass filters for this split
        filters[i * 4 + 0]->prepare(spec);
        filters[i * 4 + 0]->setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        filters[i * 4 + 2]->prepare(spec);
        filters[i * 4 + 2]->setType(juce::dsp::LinkwitzRileyFilterType::lowpass);

        // Highpass filters for this split
        filters[i * 4 + 1]->prepare(spec);
        filters[i * 4 + 1]->setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        filters[i * 4 + 3]->prepare(spec);
        filters[i * 4 + 3]->setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    }
}

void CromaSatAudioProcessor::releaseResources() {}

float CromaSatAudioProcessor::saturate(float input, int type, float drive)
{
    float x = input * (1.0f + drive * 0.1f);
    
    switch (type)
    {
        case 0: // Tube (Asymmetrical)
            return (x > 0) ? std::tanh(x) : (x / (1.0f - x * 0.5f));
        case 1: // Tape (Hysteresis-like tanh)
            return std::tanh(x * 1.2f) * 0.9f;
        case 2: // Transformer (Iron core saturation)
            return std::sin(juce::MathConstants<float>::halfPi * std::tanh(x * 0.8f));
        case 3: // Distortion (Harder clip)
            return juce::jlimit(-0.95f, 0.95f, x * 1.5f);
        case 4: // Solid State (Soft clip)
            return x / (1.0f + std::abs(x));
        case 5: // Crush (Quantization)
        {
            float bits = 4.0f + (1.0f - drive * 0.01f) * 12.0f;
            float step = std::pow(2.0f, -bits);
            return std::round(x / step) * step;
        }
    }
    return x;
}

void CromaSatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();

    // FFT Data Capture
    getNextAudioBlock(buffer);

    // Update Filter Frequencies
    for (int i = 0; i < (numBands - 1); ++i)
    {
        float freq = apvts.getRawParameterValue("crossFreq" + juce::String(i + 1))->load();
        for (int j = 0; j < 4; ++j)
            filters[i * 4 + j]->setCutoffFrequency(freq);
    }

    float inputGainMult = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputGain")->load());
    float outputGainMult = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("outputGain")->load());

    // Temporary buffers for bands
    std::array<juce::AudioBuffer<float>, numBands> bands;
    for (auto& b : bands) b.setSize(totalNumChannels, numSamples);

    // Multiband Splitting Logic (Recursive Tree)
    juce::AudioBuffer<float> signalToSplit;
    signalToSplit.makeCopyOf(buffer);
    signalToSplit.applyGain(inputGainMult);

    for (int i = 0; i < (numBands - 1); ++i)
    {
        for (int ch = 0; ch < totalNumChannels; ++ch)
        {
            auto* src = signalToSplit.getReadPointer(ch);
            auto* bLow = bands[i].getWritePointer(ch);
            auto* bHigh = (i == numBands - 2) ? bands[i+1].getWritePointer(ch) : signalToSplit.getWritePointer(ch);

            for (int s = 0; s < numSamples; ++s)
            {
                float inputSample = src[s];
                // Use separate filter instances to avoid state corruption
                bLow[s] = filters[i * 4 + (ch * 2) + 0]->processSample(ch, inputSample);
                float highSample = filters[i * 4 + (ch * 2) + 1]->processSample(ch, inputSample);
                
                // If it's not the last band, we keep "high" in signalToSplit for the next iteration
                // If it IS the last band, it goes directly to the last band buffer
                if (i < numBands - 2)
                    signalToSplit.getWritePointer(ch)[s] = highSample;
                else
                    bands[i+1].getWritePointer(ch)[s] = highSample;
            }
        }
    }

    // Applying Saturation to each band
    for (int i = 0; i < numBands; ++i)
    {
        juce::String id = "band" + juce::String(i);
        float drive = apvts.getRawParameterValue(id + "Drive")->load();
        float mix = apvts.getRawParameterValue(id + "Mix")->load() / 100.0f;
        float level = juce::Decibels::decibelsToGain(apvts.getRawParameterValue(id + "Level")->load());
        int type = (int)apvts.getRawParameterValue(id + "Type")->load();
        bool enabled = apvts.getRawParameterValue(id + "Enabled")->load() > 0.5f;

        if (!enabled)
        {
            bands[i].clear();
            continue;
        }

        for (int ch = 0; ch < totalNumChannels; ++ch)
        {
            auto* data = bands[i].getWritePointer(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                float dry = data[s];
                float saturated = saturate(dry, type, drive);
                data[s] = (dry + mix * (saturated - dry)) * level;
            }
        }
    }

    // Summing bands
    buffer.clear();
    for (int i = 0; i < numBands; ++i)
    {
        for (int ch = 0; ch < totalNumChannels; ++ch)
            buffer.addFrom(ch, 0, bands[i], ch, 0, numSamples);
    }

    // Final Output
    buffer.applyGain(outputGainMult);
}

void CromaSatAudioProcessor::getNextAudioBlock(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() > 0)
    {
        auto* channelData = buffer.getReadPointer(0);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            fifo[fifoIndex++] = channelData[i];
            if (fifoIndex == fftSize)
            {
                if (!nextFFTBlockReady)
                {
                    std::fill(fftData.begin(), fftData.end(), 0.0f);
                    std::copy(fifo.begin(), fifo.end(), fftData.begin());
                    window->multiplyWithWindowingTable(fftData.data(), fftSize);
                    forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());
                    nextFFTBlockReady = true;
                }
                fifoIndex = 0;
            }
        }
    }
}

void CromaSatAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CromaSatAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CromaSatAudioProcessor(); }
