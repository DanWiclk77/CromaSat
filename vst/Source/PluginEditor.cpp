#include "PluginProcessor.h"
#include "PluginEditor.h"

CromaSatAudioProcessorEditor::CromaSatAudioProcessorEditor (CromaSatAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), analyzer(p)
{
    addAndMakeVisible(analyzer);

    // Global Setup
    auto setupGlobal = [this](juce::Slider& s, juce::Label& l, const juce::String& name) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        addAndMakeVisible(s);
        l.setText(name, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(l);
    };

    setupGlobal(inputSlider, inputLabel, "IN");
    setupGlobal(outputSlider, outputLabel, "OUT");
    setupGlobal(globalMixSlider, globalMixLabel, "MIX");

    inputAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "inputGain", inputSlider);
    outputAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "outputGain", outputSlider);
    globalMixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "globalMix", globalMixSlider);

    // Band Setup
    for (int i = 0; i < 4; ++i)
    {
        bandButtons[i].setButtonText("BAND " + juce::String(i + 1));
        bandButtons[i].setRadioGroupId(101);
        bandButtons[i].setClickingTogglesState(true);
        bandButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey.withAlpha(0.2f));
        bandButtons[i].setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00ffff));
        bandButtons[i].setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        
        bandButtons[i].onClick = [this, i] { selectBand(i); };
        addAndMakeVisible(bandButtons[i]);
    }
    bandButtons[0].setToggleState(true, juce::sendNotification);

    // Per-Band Controls
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(driveSlider);
    driveLabel.setText("DRIVE", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);

    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(mixSlider);
    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    levelSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    levelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(levelSlider);
    levelLabel.setText("LEVEL", juce::dontSendNotification);
    levelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(levelLabel);

    typeCombo.addItemList({ "Tube", "Tape", "Transformer", "Solid State", "Distortion", "Crush" }, 1);
    addAndMakeVisible(typeCombo);
    typeLabel.setText("MODEL", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(typeLabel);

    modeCombo.addItemList({ "Stereo", "Mid", "Side" }, 1);
    addAndMakeVisible(modeCombo);
    modeLabel.setText("MODE", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modeLabel);

    // Crossover Setup
    auto setupCross = [this](juce::Slider& s, juce::Label& l, const juce::String& name, float mid) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        s.setTextValueSuffix(" Hz");
        s.setSkewFactorFromMidPoint(mid);
        addAndMakeVisible(s);
        l.setText(name, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };

    setupCross(cross1Slider, cross1Label, "X-OVER 1", 100.0f);
    setupCross(cross2Slider, cross2Label, "X-OVER 2", 1000.0f);
    setupCross(cross3Slider, cross3Label, "X-OVER 3", 5000.0f);

    cross1Attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "crossFreq1", cross1Slider);
    cross2Attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "crossFreq2", cross2Slider);
    cross3Attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "crossFreq3", cross3Slider);

    selectBand(0);

    setSize (800, 600);
    setResizable (true, true);
    setResizeLimits(600, 500, 1200, 900);
}

CromaSatAudioProcessorEditor::~CromaSatAudioProcessorEditor() {}

void CromaSatAudioProcessorEditor::selectBand(int index)
{
    selectedBand = index;
    juce::String id = "band" + juce::String(index);
    
    // Highlight selected button
    for (int i = 0; i < 4; ++i)
    {
        if (i == index)
        {
            bandButtons[i].setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00ffff));
            bandButtons[i].setColour(juce::TextButton::textColourOnId, juce::Colours::black);
            bandButtons[i].setToggleState(true, juce::dontSendNotification);
        }
        else
        {
            bandButtons[i].setToggleState(false, juce::dontSendNotification);
        }
    }

    // Explicitly reset attachments
    driveAttach.reset();
    mixAttach.reset();
    levelAttach.reset();
    typeAttach.reset();
    modeAttach.reset();

    driveAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, id + "Drive", driveSlider);
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, id + "Mix", mixSlider);
    levelAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, id + "Level", levelSlider);
    typeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, id + "Type", typeCombo);
    modeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, id + "Mode", modeCombo);
    
    repaint();
}

void CromaSatAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff0a0a0a));
    
    g.setColour(juce::Colour(0xff00ffff)); // Neon Cyan/Blue
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawText("CROMA SAT | MULTIBAND", 20, 10, 400, 30, juce::Justification::left);

    // Section Dividers with neon glow
    g.setColour(juce::Colour(0xff00ffff).withAlpha(0.2f));
    g.drawHorizontalLine(50, 0, (float)getWidth());
    g.drawHorizontalLine(200, 0, (float)getWidth());
    g.drawHorizontalLine(280, 0, (float)getWidth()); // Below crossovers
    g.drawHorizontalLine(getHeight() - 60, 0, (float)getWidth());
}

void CromaSatAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Top Bar
    area.removeFromTop(50);
    
    // Analyzer
    analyzer.setBounds(area.removeFromTop(150).reduced(10));
    
    // Crossovers
    auto crossoverArea = area.removeFromTop(80);
    auto crossWidth = crossoverArea.getWidth() / 3;
    
    auto x1 = crossoverArea.removeFromLeft(crossWidth).reduced(5);
    cross1Label.setBounds(x1.removeFromTop(20));
    cross1Slider.setBounds(x1);

    auto x2 = crossoverArea.removeFromLeft(crossWidth).reduced(5);
    cross2Label.setBounds(x2.removeFromTop(20));
    cross2Slider.setBounds(x2);

    auto x3 = crossoverArea.reduced(5);
    cross3Label.setBounds(x3.removeFromTop(20));
    cross3Slider.setBounds(x3);

    // Band Seletion
    auto bandArea = area.removeFromTop(40);
    auto buttonWidth = getWidth() / 4;
    for (int i = 0; i < 4; ++i)
        bandButtons[i].setBounds(bandArea.removeFromLeft(buttonWidth).reduced(5));

    // Per-Band Controls
    auto controlArea = area.removeFromTop(area.getHeight() - 60);
    auto subWidth = controlArea.getWidth() / 5;
    
    auto typeArea = controlArea.removeFromLeft(subWidth).reduced(10);
    typeLabel.setBounds(typeArea.removeFromTop(30));
    typeCombo.setBounds(typeArea.removeFromTop(30));

    auto mModeArea = controlArea.removeFromLeft(subWidth).reduced(10);
    modeLabel.setBounds(mModeArea.removeFromTop(30));
    modeCombo.setBounds(mModeArea.removeFromTop(30));

    auto dArea = controlArea.removeFromLeft(subWidth);
    driveLabel.setBounds(dArea.removeFromTop(30));
    driveSlider.setBounds(dArea);

    auto lArea = controlArea.removeFromLeft(subWidth);
    levelLabel.setBounds(lArea.removeFromTop(30));
    levelSlider.setBounds(lArea);

    auto mArea = controlArea;
    mixLabel.setBounds(mArea.removeFromTop(30));
    mixSlider.setBounds(mArea);

    // Bottom Global Bar
    auto bottomArea = getLocalBounds().removeFromBottom(60).reduced(10);
    auto globalWidth = bottomArea.getWidth() / 3;
    
    auto inArea = bottomArea.removeFromLeft(globalWidth);
    inputLabel.setBounds(inArea.removeFromLeft(30));
    inputSlider.setBounds(inArea);

    auto outArea = bottomArea.removeFromLeft(globalWidth);
    outputLabel.setBounds(outArea.removeFromLeft(40));
    outputSlider.setBounds(outArea);

    auto mixArea = bottomArea;
    globalMixLabel.setBounds(mixArea.removeFromLeft(40));
    globalMixSlider.setBounds(mixArea);
}
