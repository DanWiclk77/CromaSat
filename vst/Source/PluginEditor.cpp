#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

CromaSatAudioProcessorEditor::CromaSatAudioProcessorEditor (CromaSatAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      webView (juce::WebBrowserComponent::Options()
                .withResourceProvider ([this] (const juce::String& url) { return uiProvider.getResource (url); })
                .withWinWebView2Storage (juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("CromaSat_WebView2")))
{
    addAndMakeVisible (webView);
    
    // Load UI from the virtual internal server
    webView.goToURL ("http://ui.local/");
    
    setSize (1000, 650);
    setResizable (true, true);
}

CromaSatAudioProcessorEditor::~CromaSatAudioProcessorEditor() {}

void CromaSatAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void CromaSatAudioProcessorEditor::resized()
{
    webView.setBounds (getLocalBounds());
}
