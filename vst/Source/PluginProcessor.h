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
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // FFT Data for Spectrum Analyzer (Dual)
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    void getNextAudioBlock(const juce::AudioBuffer<float>& buffer, bool isInput);
    
    std::array<float, fftSize> fftDataIn, fftDataOut;
    std::atomic<bool> nextFFTBlockReadyIn { false };
    std::atomic<bool> nextFFTBlockReadyOut { false };
    juce::CriticalSection fftLock;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Saturation Algorithms
    float saturate(float input, int type, float drive);

    // Multiband Crossovers
    static constexpr int numBands = 4;
    static constexpr int maxChans = 8; 
    // (numBands - 1) split points * 2 types (LP/HP) * maxChans = 48 filters for 8 channels
    std::array<std::unique_ptr<juce::dsp::LinkwitzRileyFilter<float>>, (numBands - 1) * 2 * maxChans> filters;
    
    // Pre-allocated buffers to avoid allocations in processBlock
    std::array<juce::AudioBuffer<float>, numBands> bandBuffers;
    juce::AudioBuffer<float> dryCopy;
    
    // Parameter Smoothing
    struct BandSettings {
        juce::LinearSmoothedValue<float> drive, mix, level;
    };
    std::array<BandSettings, numBands> smoothedBandSettings;
    std::array<juce::LinearSmoothedValue<float>, numBands - 1> smoothedCrossovers;
    juce::LinearSmoothedValue<float> smoothedInputGain, smoothedOutputGain, smoothedGlobalMix;

    // FFT bits
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    std::array<float, fftSize> fifoIn, fifoOut;
    int fifoIndexIn = 0, fifoIndexOut = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CromaSatAudioProcessor)
};
