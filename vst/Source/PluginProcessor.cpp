#include "PluginProcessor.h"
#include "PluginEditor.h"

CromaSatAudioProcessor::CromaSatAudioProcessor()
     : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{}

CromaSatAudioProcessor::~CromaSatAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout CromaSatAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "inputGain", 1 }, "Input Gain", -24.0f, 24.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "globalMix", 1 }, "Global Mix", 0.0f, 100.0f, 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "drive", 1 }, "Drive", 0.0f, 100.0f, 0.0f));

    return { params.begin(), params.end() };
}

void CromaSatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {}
void CromaSatAudioProcessor::releaseResources() {}

void CromaSatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get parameters
    auto gainRaw = apvts.getRawParameterValue ("inputGain")->load();
    auto driveRaw = apvts.getRawParameterValue ("drive")->load();
    auto mixRaw = apvts.getRawParameterValue ("globalMix")->load() / 100.0f;

    float gainMult = juce::Decibels::decibelsToGain (gainRaw);
    float driveMult = 1.0f + (driveRaw * 0.1f);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float input = channelData[sample] * gainMult;
            float dry = input;
            
            // Soft clipping (atan approximation)
            float x = input * driveMult;
            float saturated = x / (1.0f + std::abs(x));
            
            channelData[sample] = dry + mixRaw * (saturated - dry);
        }
    }
}

juce::AudioProcessorEditor* CromaSatAudioProcessor::createEditor()
{
    return new CromaSatAudioProcessorEditor (*this);
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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CromaSatAudioProcessor();
}
