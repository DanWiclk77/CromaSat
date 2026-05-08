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
        g.fillAll(juce::Colours::black);
        
        auto drawSpectrum = [&](const std::array<float, CromaSatAudioProcessor::fftSize>& fftData, juce::Colour color, float alpha)
        {
            int numPoints = 256; 
            auto width = (float)getWidth();
            auto height = (float)getHeight();

            juce::Path path;
            bool first = true;

            for (int i = 0; i < numPoints; ++i)
            {
                auto x = juce::jmap((float)i, 0.0f, (float)numPoints, 0.0f, width);
                float mag = fftData[i] / (float)CromaSatAudioProcessor::fftSize;
                float level = 0.0f;
                
                if (mag > 0.000001f) {
                    float db = juce::Decibels::gainToDecibels(mag);
                    level = juce::jmap(db, -80.0f, 6.0f, 0.0f, 1.0f);
                }
                
                auto y = juce::jmap(juce::jlimit(0.0f, 1.0f, level), 0.0f, 1.0f, height, 0.0f);
                
                if (first) {
                    path.startNewSubPath(x, y);
                    first = false;
                } else {
                    path.lineTo(x, y);
                }
            }
            
            // Draw Fill
            juce::Path fillPath = path;
            fillPath.lineTo(width, height);
            fillPath.lineTo(0, height);
            fillPath.closeSubPath();
            
            g.setColour(color.withAlpha(alpha * 0.2f));
            g.fillPath(fillPath);
            
            // Draw Glow Line
            g.setColour(color.withAlpha(alpha * 0.5f));
            g.strokePath(path, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            
            // Draw main line
            g.setColour(color.withAlpha(alpha));
            g.strokePath(path, juce::PathStrokeType(1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        };

        // Original Signal - Neon Blue/Cyan
        if (processor.nextFFTBlockReadyIn) {
            drawSpectrum(processor.fftDataIn, juce::Colour(0xff00ffff), 0.7f);
        }

        // Processed Signal (Harmonics) - Neon Purple/Magenta
        if (processor.nextFFTBlockReadyOut) {
            drawSpectrum(processor.fftDataOut, juce::Colour(0xffff00ff), 0.9f);
            processor.nextFFTBlockReadyIn = false;
            processor.nextFFTBlockReadyOut = false;
        }
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
