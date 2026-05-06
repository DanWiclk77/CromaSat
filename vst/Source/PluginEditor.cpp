#include "PluginProcessor.h"
#include "PluginEditor.h"

CromaSatAudioProcessorEditor::CromaSatAudioProcessorEditor (CromaSatAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible (webView);
    
    // In production, we point this to the loaded BinaryData (React build)
    // webView.goToURL ("file:///..."); 
    
    // For the UI preview, we set a fixed size
    setSize (800, 600);
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
