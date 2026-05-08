#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

CromaSatAudioProcessorEditor::CromaSatAudioProcessorEditor (CromaSatAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible (webView);
    
    if (BinaryData::index_htmlSize > 0)
    {
        auto indexHtml = juce::String::createStringFromData (BinaryData::index_html, BinaryData::index_htmlSize);
        webView.goToURL ("data:text/html;base64," + juce::Base64::toBase64 (indexHtml.toRawUTF8(), (size_t) indexHtml.getNumBytesAsUTF8()));
    }
    else
    {
        webView.goToURL ("data:text/html,<html><body style='background:#111;color:#f44;padding:20px;font-family:sans-serif'><h1>UI Data Missing</h1><p>BinaryData::index_html size is 0.</p><p>Check if npm run build generated dist/index.html before CMake.</p></body></html>");
    }
    
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
