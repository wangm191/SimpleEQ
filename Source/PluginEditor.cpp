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
peakThreeFreqSliderAttachment(audioProcessor.apvts, "PeakThree Freq", peakThreeFreqSlider),
peakThreeGainSliderAttachment(audioProcessor.apvts, "PeakThree Gain", peakThreeGainSlider),
peakThreeQualitySliderAttachment(audioProcessor.apvts, "PeakThree Quality", peakThreeQualitySlider),
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
    
    setSize (800, 600);
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
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.125);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.142857);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.6 ));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.6 ));
    highCutSlopeSlider.setBounds(highCutArea);
    
    auto peakOneArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto peakTwoArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);

    peakOneFreqSlider.setBounds(peakOneArea.removeFromTop(peakOneArea.getHeight() * 0.5 ));
    peakOneGainSlider.setBounds(peakOneArea.removeFromLeft(peakOneArea.getWidth() * 0.5 ));
    peakOneQualitySlider.setBounds(peakOneArea);

    peakTwoFreqSlider.setBounds(peakTwoArea.removeFromTop(peakTwoArea.getHeight() * 0.5));
    peakTwoGainSlider.setBounds(peakTwoArea.removeFromLeft(peakTwoArea.getWidth() * 0.5));
    peakTwoQualitySlider.setBounds(peakTwoArea);
    
    peakThreeFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakThreeGainSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.5));
    peakThreeQualitySlider.setBounds(bounds);
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
        &peakThreeFreqSlider,
        &peakThreeGainSlider,
        &peakThreeQualitySlider,
        &lowCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutFreqSlider,
        &highCutSlopeSlider
    };
}
