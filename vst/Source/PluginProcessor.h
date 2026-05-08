#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <memory>
#include <atomic>

class CromaSatAudioProcessor : public juce::AudioProcessor
{
public:
    CromaSatAudioProcessor();
    ~CromaSatAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Croma Sat"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // FFT Data for Spectrum Analyzer
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    void getNextAudioBlock(const juce::AudioBuffer<float>& buffer);
    std::array<float, 2 * fftSize> fftData;
    std::atomic<bool> nextFFTBlockReady { false };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Saturation Algorithms
    float saturate(float input, int type, float drive);

    // Multiband Crossovers
    static constexpr int numBands = 6;
    // 5 split points * (2 channels L/R) * (2 types LP/HP) = 20 filters
    std::array<std::unique_ptr<juce::dsp::LinkwitzRileyFilter<float>>, (numBands - 1) * 4> filters;
    
    // FFT bits
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    std::array<float, fftSize> fifo;
    int fifoIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CromaSatAudioProcessor)
};
