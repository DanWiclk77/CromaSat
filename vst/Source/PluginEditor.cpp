#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

CromaSatAudioProcessorEditor::CromaSatAudioProcessorEditor (CromaSatAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible (webView);
    
    // Serve the bundled React app
    auto indexHtml = juce::String::createStringFromData(BinaryData::index_html, BinaryData::index_htmlSize);
    webView.goToURL("data:text/html;base64," + juce::Base64::toBase64(indexHtml.toRawUTF8(), indexHtml.getNumBytesAsUTF8()));
    
    setSize (1000, 650);
    setResizable(true, true);
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
