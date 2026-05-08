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
    struct UIProvider : public juce::WebBrowserComponent::ResourceProvider
    {
        std::optional<ResourceResponse> getResource (const juce::String& url) override
        {
            if (url.endsWith ("index.html") || url == "http://ui.local/")
            {
                return ResourceResponse { 
                    juce::MemoryBlock (BinaryData::index_html, BinaryData::index_htmlSize), 
                    "text/html" 
                };
            }
            return std::nullopt;
        }
    };

    UIProvider uiProvider;
    juce::WebBrowserComponent webView;
    CromaSatAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CromaSatAudioProcessorEditor)
};
