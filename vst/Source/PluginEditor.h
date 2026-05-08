#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    SpectrumAnalyzer(CromaSatAudioProcessor& p) : processor(p) { startTimerHz(30); }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::orange.withAlpha(0.5f));
        
        auto& fftData = processor.fftData;
        int numPoints = 256; 
        auto width = (float)getWidth();
        auto height = (float)getHeight();

        juce::Path p;
        p.startNewSubPath(0, height);

        for (int i = 0; i < numPoints; ++i)
        {
            auto x = juce::jmap((float)i, 0.0f, (float)numPoints, 0.0f, width);
            auto val = juce::FloatVectorOperations::findMaximum(fftData.data() + i * 4, 4);
            auto y = juce::jmap(juce::jlimit(0.0f, 1.0f, val * 10.0f), 0.0f, 1.0f, height, 0.0f);
            p.lineTo(x, y);
        }
        
        p.lineTo(width, height);
        p.closeSubPath();
        g.fillPath(p);
    }
    
    void timerCallback() override { repaint(); }

private:
    CromaSatAudioProcessor& processor;
};

class CromaSatAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    CromaSatAudioProcessorEditor (CromaSatAudioProcessor&);
    ~CromaSatAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CromaSatAudioProcessor& audioProcessor;
    SpectrumAnalyzer analyzer;

    // Global
    juce::Slider inputSlider, outputSlider, globalMixSlider;
    juce::Label inputLabel, outputLabel, globalMixLabel;
    
    // Bands
    int selectedBand = 0;
    std::array<juce::TextButton, 6> bandButtons;
    
    juce::Slider driveSlider, mixSlider, levelSlider;
    juce::ComboBox typeCombo;
    juce::Label driveLabel, mixLabel, levelLabel, typeLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttach, outputAttach, globalMixAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttach, mixAttach, levelAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttach;

    void selectBand(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CromaSatAudioProcessorEditor)
};
