#include "PluginProcessor.h"
#include "PluginEditor.h"

CromaSatAudioProcessorEditor::CromaSatAudioProcessorEditor (CromaSatAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Configure Sliders
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        addAndMakeVisible(slider);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupSlider(inputGainSlider, inputGainLabel, "INPUT");
    setupSlider(driveSlider, driveLabel, "DRIVE");
    setupSlider(mixSlider, mixLabel, "MIX");

    // Attachments
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "inputGain", inputGainSlider);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drive", driveSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "globalMix", mixSlider);

    setSize (600, 400);
    setResizable (true, true);
    setResizeLimits(400, 300, 1000, 800);
}

CromaSatAudioProcessorEditor::~CromaSatAudioProcessorEditor() {}

void CromaSatAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background
    auto backgroundGradient = juce::ColourGradient(
        juce::Colour(0xff1a1a1a), 0, 0,
        juce::Colour(0xff0d0d0d), 0, (float)getHeight(), false);
    g.setGradientFill(backgroundGradient);
    g.fillAll();

    // Title
    g.setColour(juce::Colours::orange);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("CROMA SAT", getLocalBounds().removeFromTop(60), juce::Justification::centred);
    
    // Bottom Bar
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(0, getHeight() - 40, getWidth(), 40);
    g.setColour(juce::Colours::grey);
    g.setFont(12.0f);
    g.drawText("V1.0.0 | Native Edition", 0, getHeight() - 40, getWidth(), 40, juce::Justification::centred);
}

void CromaSatAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(80); // Title space
    area.removeFromBottom(60); // Bottom bar space

    auto sliderWidth = area.getWidth() / 3;
    
    auto inputArea = area.removeFromLeft(sliderWidth);
    inputGainLabel.setBounds(inputArea.removeFromTop(30));
    inputGainSlider.setBounds(inputArea.reduced(10));

    auto driveArea = area.removeFromLeft(sliderWidth);
    driveLabel.setBounds(driveArea.removeFromTop(30));
    driveSlider.setBounds(driveArea.reduced(20)); // Drive is bigger visually in center

    auto mixArea = area;
    mixLabel.setBounds(mixArea.removeFromTop(30));
    mixSlider.setBounds(mixArea.reduced(10));
}
