#include "PluginProcessor.h"
#include "PluginEditor.h"

CromaSatAudioProcessor::CromaSatAudioProcessor()
     : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{}

CromaSatAudioProcessor::~CromaSatAudioProcessor() {}

void CromaSatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {}
void CromaSatAudioProcessor::releaseResources() {}

void CromaSatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Basic saturation DSP would go here in C++ for performance
    // For now, this is a bridge to the React-managed engine or a simple pass-through
}

juce::AudioProcessorEditor* CromaSatAudioProcessor::createEditor()
{
    return new CromaSatAudioProcessorEditor (*this);
}

void CromaSatAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
void CromaSatAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CromaSatAudioProcessor();
}
