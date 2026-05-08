#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    SpectrumAnalyzer(CromaSatAudioProcessor& p) : processor(p) { startTimerHz(30); }
    
    void paint(juce::Graphics& g) override
    {
        if (!processor.nextFFTBlockReady)
            return;

        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::orange.withAlpha(0.5f));
        
        auto& fftData = processor.fftData;
        int numPoints = 256; 
        auto width = (float)getWidth();
        auto height = (float)getHeight();

        juce::Path spectrumPath;
        spectrumPath.startNewSubPath(0, height);

        for (int i = 0; i < numPoints; ++i)
        {
            auto x = juce::jmap((float)i, 0.0f, (float)numPoints, 0.0f, width);
            
            // Normalize FFT magnitudes and use a more sensitive mapping
            float mag = fftData[i] / (float)CromaSatAudioProcessor::fftSize;
            float level = 0.0f;
            
            if (mag > 0.000001f) {
                float db = juce::Decibels::gainToDecibels(mag);
                level = juce::jmap(db, -100.0f, 12.0f, 0.0f, 1.0f);
            }
            
            auto y = juce::jmap(juce::jlimit(0.0f, 1.0f, level), 0.0f, 1.0f, height, 0.0f);
            
            if (i == 0) spectrumPath.startNewSubPath(0, y);
            else spectrumPath.lineTo(x, y);
        }
        
        spectrumPath.lineTo(width, height);
        spectrumPath.lineTo(0, height);
        spectrumPath.closeSubPath();
        
        g.setColour(juce::Colours::orange.withAlpha(0.3f));
        g.fillPath(spectrumPath);
        g.setColour(juce::Colours::orange);
        g.strokePath(spectrumPath, juce::PathStrokeType(1.0f));

        processor.nextFFTBlockReady = false;
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
    std::array<juce::TextButton, 4> bandButtons;
    
    juce::Slider driveSlider, mixSlider, levelSlider;
    juce::ComboBox typeCombo, modeCombo;
    juce::Label driveLabel, mixLabel, levelLabel, typeLabel, modeLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttach, outputAttach, globalMixAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttach, mixAttach, levelAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttach, modeAttach;

    // Crossovers
    juce::Slider cross1Slider, cross2Slider, cross3Slider;
    juce::Label cross1Label, cross2Label, cross3Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cross1Attach, cross2Attach, cross3Attach;

    void selectBand(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CromaSatAudioProcessorEditor)
};
