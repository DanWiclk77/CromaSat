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

    for (auto& f : postFilters)
        f = std::make_unique<juce::dsp::LinkwitzRileyFilter<float>>();

    for (int i = 0; i < numBands; ++i)
        bandBuffers[i] = juce::AudioBuffer<float>(2, 1024); // Initial hint, will resize in prepareToPlay

    fftDataIn.fill(0);
    fftDataOut.fill(0);
    fifoIn.fill(0);
    fifoOut.fill(0);
}

CromaSatAudioProcessor::~CromaSatAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout CromaSatAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "inputGain", 1 }, "Input Gain", -24.0f, 24.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "globalMix", 1 }, "Global Mix", 0.0f, 100.0f, 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "outputGain", 1 }, "Output Gain", -24.0f, 24.0f, 0.0f));

    // Crossover Frequencies
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "crossFreq1", 1 }, "Crossover 1", 20.0f, 500.0f, 200.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "crossFreq2", 1 }, "Crossover 2", 500.0f, 2000.0f, 1000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "crossFreq3", 1 }, "Crossover 3", 2000.0f, 15000.0f, 5000.0f));

    // Per-band Parameters
    for (int i = 0; i < numBands; ++i)
    {
        juce::String id = "band" + juce::String(i);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id + "Drive", 1 }, "Drive " + juce::String(i+1), 0.0f, 100.0f, 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id + "Mix", 1 }, "Mix " + juce::String(i+1), 0.0f, 100.0f, 100.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id + "Level", 1 }, "Level " + juce::String(i+1), -12.0f, 12.0f, 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id + "Type", 1 }, "Type " + juce::String(i+1), 
            juce::StringArray { "Tube", "Tape", "Transformer", "Solid State", "Distortion", "Crush" }, 0));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id + "Mode", 1 }, "Mode " + juce::String(i+1), 
            juce::StringArray { "Stereo", "Mid", "Side" }, 0));
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

    dryCopy.setSize(spec.numChannels, samplesPerBlock);

    for (int i = 0; i < numBands; ++i)
        bandBuffers[i].setSize(spec.numChannels, samplesPerBlock);

    juce::dsp::ProcessSpec monoSpec;
    monoSpec.sampleRate = sampleRate;
    monoSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    monoSpec.numChannels = 1;

    for (int i = 0; i < (numBands - 1); ++i)
    {
        for (int ch = 0; ch < maxChans; ++ch)
        {
            // LP and HP for each channel
            int lpIndex = i * (2 * maxChans) + (ch * 2) + 0;
            int hpIndex = i * (2 * maxChans) + (ch * 2) + 1;
            
            filters[lpIndex]->prepare(monoSpec);
            filters[lpIndex]->setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
            
            filters[hpIndex]->prepare(monoSpec);
            filters[hpIndex]->setType(juce::dsp::LinkwitzRileyFilterType::highpass);

            postFilters[lpIndex]->prepare(monoSpec);
            postFilters[lpIndex]->setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
            
            postFilters[hpIndex]->prepare(monoSpec);
            postFilters[hpIndex]->setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        }

        smoothedCrossovers[i].reset(sampleRate, 0.05);
        smoothedCrossovers[i].setCurrentAndTargetValue(apvts.getRawParameterValue("crossFreq" + juce::String(i + 1))->load());
    }

    smoothedInputGain.reset(sampleRate, 0.05);
    smoothedInputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputGain")->load()));
    
    smoothedOutputGain.reset(sampleRate, 0.05);
    smoothedOutputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("outputGain")->load()));
    
    smoothedGlobalMix.reset(sampleRate, 0.05);
    smoothedGlobalMix.setCurrentAndTargetValue(apvts.getRawParameterValue("globalMix")->load() / 100.0f);

    for (int i = 0; i < numBands; ++i) {
        juce::String id = "band" + juce::String(i);
        smoothedBandSettings[i].drive.reset(sampleRate, 0.05);
        smoothedBandSettings[i].drive.setCurrentAndTargetValue(apvts.getRawParameterValue(id + "Drive")->load());
        
        smoothedBandSettings[i].mix.reset(sampleRate, 0.05);
        smoothedBandSettings[i].mix.setCurrentAndTargetValue(apvts.getRawParameterValue(id + "Mix")->load() / 100.0f);
        
        smoothedBandSettings[i].level.reset(sampleRate, 0.05);
        smoothedBandSettings[i].level.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue(id + "Level")->load()));
    }
}

void CromaSatAudioProcessor::releaseResources() {}

float CromaSatAudioProcessor::saturate(float input, int type, float drive)
{
    if (drive <= 0.01f) return input;

    float x = input * (1.0f + drive * 0.15f);
    float saturated = input;
    
    switch (type)
    {
        case 0: // Tube (Soft, Stable Asymmetrical)
            saturated = (x > 0) ? std::tanh(x) : std::atan(x * 0.8f);
            break;
        case 1: // Tape (Saturation)
            saturated = std::tanh(x * 1.5f);
            break;
        case 2: // Transformer
            saturated = std::sin(juce::jlimit(-1.0f, 1.0f, x * 0.8f) * juce::MathConstants<float>::halfPi);
            break;
        case 3: // Distortion (Hard)
            saturated = juce::jlimit(-0.9f, 0.9f, x * 2.0f);
            break;
        case 4: // Solid State (Soft)
            saturated = std::atan(x) * (2.0f / juce::MathConstants<float>::pi);
            break;
        case 5: // Crush (Quantization)
        {
            float bits = 2.0f + (1.0f - drive * 0.01f) * 14.0f;
            float step = std::pow(2.0f, -bits);
            saturated = std::round(x / step) * step;
            break;
        }
    }

    // Blend based on drive to ensure smooth transition from clean
    float blend = juce::jlimit(0.0f, 1.0f, drive * 0.1f);
    
    // Perceived volume boost for "analog" character
    float makeup = 1.0f + (drive * 0.05f); // Up to ~0.5dB to 1dB boost at full drive 
    
    return (input + blend * (saturated - input)) * makeup;
}

void CromaSatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();

    // FFT Data Capture (Input)
    getNextAudioBlock(buffer, true);

    // Update Smoothing Targets
    smoothedInputGain.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputGain")->load()));
    smoothedOutputGain.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("outputGain")->load()));
    smoothedGlobalMix.setTargetValue(apvts.getRawParameterValue("globalMix")->load() / 100.0f);

    for (int i = 0; i < (numBands - 1); ++i) {
        float freq = apvts.getRawParameterValue("crossFreq" + juce::String(i + 1))->load();
        smoothedCrossovers[i].setTargetValue(freq);
        
        float currentFreq = smoothedCrossovers[i].getNextValue();
        
        // Update filter coefficients once per block (more stable/efficient)
        // Ensure freq is within valid range for sample rate
        currentFreq = juce::jlimit(20.0f, (float)getSampleRate() * 0.49f, currentFreq);
        
        for (int j = 0; j < 2 * maxChans; ++j) {
            filters[i * (2 * maxChans) + j]->setCutoffFrequency(currentFreq);
            postFilters[i * (2 * maxChans) + j]->setCutoffFrequency(currentFreq);
        }
    }

    for (int i = 0; i < numBands; ++i) {
        juce::String id = "band" + juce::String(i);
        smoothedBandSettings[i].drive.setTargetValue(apvts.getRawParameterValue(id + "Drive")->load());
        smoothedBandSettings[i].mix.setTargetValue(apvts.getRawParameterValue(id + "Mix")->load() / 100.0f);
        smoothedBandSettings[i].level.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue(id + "Level")->load()));
    }

    // Pre-allocate dry copy safely
    int channelsToCopy = std::min(totalNumChannels, dryCopy.getNumChannels());
    for (int ch = 0; ch < channelsToCopy; ++ch)
        dryCopy.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    int actualChannels = std::min((int)totalNumChannels, maxChans);

    for (int s = 0; s < numSamples; ++s)
    {
        float inGain = smoothedInputGain.getNextValue();
        
        for (int ch = 0; ch < actualChannels; ++ch)
        {
            float inputSample = buffer.getSample(ch, s) * inGain;
            float signalToSplit = inputSample;

            for (int i = 0; i < (numBands - 1); ++i)
            {
                // Each filter is mono, so we use 0 as channel index for processSample
                int lpIndex = i * (2 * maxChans) + (ch * 2) + 0;
                int hpIndex = i * (2 * maxChans) + (ch * 2) + 1;

                float low  = filters[lpIndex]->processSample(0, signalToSplit);
                float high = filters[hpIndex]->processSample(0, signalToSplit);
                
                bandBuffers[i].setSample(ch, s, low);
                signalToSplit = high; 
                
                if (i == (numBands - 2))
                    bandBuffers[i+1].setSample(ch, s, high);
            }
        }
    }

    // Processing each band (Buffer-based as it's easier and Algorithms are per-sample anyway)
    for (int i = 0; i < numBands; ++i)
    {
        juce::String id = "band" + juce::String(i);
        int type = (int)apvts.getRawParameterValue(id + "Type")->load();
        int mode = (int)apvts.getRawParameterValue(id + "Mode")->load();
        bool enabled = apvts.getRawParameterValue(id + "Enabled")->load() > 0.5f;

        if (!enabled)
        {
            bandBuffers[i].clear();
            continue;
        }

        // We use samples from start of block, but need to sync smoothing
        // Let's reset smoothing to start of block if we want to be perfect, 
        // but it's better to just process them sample by sample here too.
        
        auto* left = bandBuffers[i].getWritePointer(0);
        auto* right = (totalNumChannels >= 2 && bandBuffers[i].getNumChannels() >= 2) ? bandBuffers[i].getWritePointer(1) : nullptr;

        for (int s = 0; s < numSamples; ++s)
        {
            float d = smoothedBandSettings[i].drive.getNextValue();
            float m = smoothedBandSettings[i].mix.getNextValue();
            float l = smoothedBandSettings[i].level.getNextValue();

            if (right && (mode == 1 || mode == 2))
            {
                float mid = (left[s] + right[s]) * 0.5f;
                float side = (left[s] - right[s]) * 0.5f;

                if (mode == 1) {
                    float satM = saturate(mid, type, d);
                    mid = (mid + m * (satM - mid)) * l;
                } else {
                    float satS = saturate(side, type, d);
                    side = (side + m * (satS - side)) * l;
                }
                left[s] = mid + side;
                right[s] = mid - side;
            }
            else
            {
                for (int ch = 0; ch < actualChannels; ++ch)
                {
                    float* data = bandBuffers[i].getWritePointer(ch);
                    float dry = data[s];
                    float sat = saturate(dry, type, d);
                    data[s] = (dry + m * (sat - dry)) * l;
                }
            }
        }

        // Post-Filtering to contain harmonics within the band
        for (int ch = 0; ch < actualChannels; ++ch)
        {
            // If i > 0, we re-apply High-Pass at split i-1 to remove low-frequency artifacts
            if (i > 0)
            {
                int hpIndex = (i - 1) * (2 * maxChans) + (ch * 2) + 1;
                for (int s = 0; s < numSamples; ++s)
                {
                    float sample = bandBuffers[i].getSample(ch, s);
                    bandBuffers[i].setSample(ch, s, postFilters[hpIndex]->processSample(0, sample));
                }
            }

            // If i < numBands - 1, we re-apply Low-Pass at split i to remove high-frequency harmonics
            if (i < numBands - 1)
            {
                int lpIndex = i * (2 * maxChans) + (ch * 2) + 0;
                for (int s = 0; s < numSamples; ++s)
                {
                    float sample = bandBuffers[i].getSample(ch, s);
                    bandBuffers[i].setSample(ch, s, postFilters[lpIndex]->processSample(0, sample));
                }
            }
        }
    }

    // Summing bands
    buffer.clear();
    for (int ch = 0; ch < totalNumChannels; ++ch)
    {
        for (int i = 0; i < numBands; ++i)
        {
            if (ch < bandBuffers[i].getNumChannels())
                buffer.addFrom(ch, 0, bandBuffers[i], ch, 0, numSamples);
        }
    }

    // Global Mix and Output Gain
    for (int s = 0; s < numSamples; ++s)
    {
        float gMix = smoothedGlobalMix.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();
        
        for (int ch = 0; ch < totalNumChannels; ++ch)
        {
            float dry = dryCopy.getSample(ch, s);
            float wet = buffer.getSample(ch, s);
            buffer.setSample(ch, s, (dry + gMix * (wet - dry)) * outGain);
        }
    }

    // FFT Data Capture (Output)
    getNextAudioBlock(buffer, false);
}

void CromaSatAudioProcessor::getNextAudioBlock(const juce::AudioBuffer<float>& buffer, bool isInput)
{
    if (buffer.getNumChannels() > 0)
    {
        auto* channelData = buffer.getReadPointer(0);
        auto& fifo = isInput ? fifoIn : fifoOut;
        auto& fifoIndex = isInput ? fifoIndexIn : fifoIndexOut;
        auto& fftData = isInput ? fftDataIn : fftDataOut;
        auto& readyFlag = isInput ? nextFFTBlockReadyIn : nextFFTBlockReadyOut;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            fifo[fifoIndex++] = channelData[i];
            if (fifoIndex == fftSize)
            {
                if (!readyFlag)
                {
                    const juce::ScopedLock sl(fftLock);
                    std::fill(fftData.begin(), fftData.end(), 0.0f);
                    std::copy(fifo.begin(), fifo.end(), fftData.begin());
                    window->multiplyWithWindowingTable(fftData.data(), fftSize);
                    forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());
                    readyFlag = true;
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

juce::AudioProcessorEditor* CromaSatAudioProcessor::createEditor()
{
    return new CromaSatAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CromaSatAudioProcessor(); }
