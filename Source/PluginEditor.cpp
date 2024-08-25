/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
peakOneFreqSliderAttachment(audioProcessor.apvts, "PeakOne Freq", peakOneFreqSlider),
peakOneGainSliderAttachment(audioProcessor.apvts, "PeakOne Gain", peakOneGainSlider),
peakOneQualitySliderAttachment(audioProcessor.apvts, "PeakOne Quality", peakOneQualitySlider),
peakTwoFreqSliderAttachment(audioProcessor.apvts, "PeakTwo Freq", peakTwoFreqSlider),
peakTwoGainSliderAttachment(audioProcessor.apvts, "PeakTwo Gain", peakTwoGainSlider),
peakTwoQualitySliderAttachment(audioProcessor.apvts, "PeakTwo Quality", peakTwoQualitySlider),
lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
lowCutSlopeSliderAttachent(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for ( auto comp : getComps() )
    {
        addAndMakeVisible(comp);
    }
    
    setSize (600, 400);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.4);
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.20);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.25);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.6 ));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.6 ));
    highCutSlopeSlider.setBounds(highCutArea);
    
    auto peakOneArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);

    peakOneFreqSlider.setBounds(peakOneArea.removeFromTop(bounds.getHeight() * 0.5 ));
    peakOneGainSlider.setBounds(peakOneArea.removeFromLeft(bounds.getWidth() * 0.5 ));
    peakOneQualitySlider.setBounds(peakOneArea);

    peakTwoFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakTwoGainSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.5));
    peakTwoQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakOneFreqSlider,
        &peakOneGainSlider,
        &peakOneQualitySlider,
        &peakTwoFreqSlider,
        &peakTwoGainSlider,
        &peakTwoQualitySlider,
        &lowCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutFreqSlider,
        &highCutSlopeSlider
    };
}
