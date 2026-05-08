#pragma once
#include "PluginProcessor.h"

class CromaSatAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    CromaSatAudioProcessorEditor (CromaSatAudioProcessor&);
    ~CromaSatAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::WebBrowserComponent webView;
    CromaSatAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CromaSatAudioProcessorEditor)
};
