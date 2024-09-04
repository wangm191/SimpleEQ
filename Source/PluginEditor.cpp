/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics & g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider & slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    g.setColour(Colours::slateblue);
    g.fillEllipse(bounds);
    
    g.setColour(Colours::springgreen);
    g.drawEllipse(bounds, 1.f);
    
    if ( auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)  )
    {
        auto center = bounds.getCentre();
        Path p;
        
        Rectangle<float> rec;
        rec.setLeft(center.getX() - 2);
        rec.setRight(center.getX() + 2);
        rec.setTop(bounds.getY() + 2);
        rec.setBottom(center.getY() - rswl->getTextHeight() * 2.1);
        
        p.addRoundedRectangle(rec, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngleRadians = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle , rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngleRadians, center.getX(), center.getY()));
        
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        rec.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        rec.setCentre(center);
        
       g.setColour(Colours::transparentBlack);
        g.fillRect(rec);
        
        g.setColour(Colours::white);
        g.drawFittedText(text, rec.toNearestInt(), juce::Justification::centred, 1);
    }
};
//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAngle = degreesToRadians(180.f + 45.f);
    auto endAngle = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
//  outlines of the slider boxes 
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAngle,
                                      endAngle,
                                      *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() / 2;
    
    juce::Rectangle<int> rec;
    rec.setSize(size, size);
    rec.setCentre(bounds.getCentreX(), 0);
    rec.setY(20);

    return rec;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if ( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    //bool addK = false;
    
    if ( auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) )
    {
        float value = getValue();

//        if (value > 999.f )
//        {
//            // divide by 1000 so 1000Hz becomes 1.00kHz
//            value /= 1000.f; // 1001 / 1000 = 1.001: we just want to see 2 decimal places
//            addK = true;
//        }

        str = juce::String(value);
    }
    else {
        jassertfalse;
    }

    if ( suffix.isNotEmpty() )
    {
        str << " ";
//        if ( addK )
//            str << "k";
        
        str << suffix;
    }
    
    return str;
}
//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for ( auto param : params )
    {
        param ->addListener(this);
    }
    
    startTimer(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for ( auto param : params )
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    if ( parametersChanged.compareAndSetBool(false, true) )
    {
        //update the monochain
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakOneCoefficients = makePeakOneFilter(chainSettings, audioProcessor.getSampleRate());
        auto peakTwoCoefficients = makePeakTwoFilter(chainSettings, audioProcessor.getSampleRate());
        auto peakThreeCoefficients = makePeakThreeFilter(chainSettings, audioProcessor.getSampleRate());
        
        updateCoefficients(monoChain.get<ChainPositions::PeakOne>().coefficients, peakOneCoefficients);
        updateCoefficients(monoChain.get<ChainPositions::PeakTwo>().coefficients, peakTwoCoefficients);
        updateCoefficients(monoChain.get<ChainPositions::PeakThree>().coefficients, peakThreeCoefficients);
        
        auto lowCutCoefficients = makeLowCutFiler(chainSettings, audioProcessor.getSampleRate());
        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
        
        updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
        updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
        
        //signal a repaint
        repaint();
    }
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    auto responseArea = getLocalBounds();
    auto responseWidth = responseArea.getWidth();
    
    auto& lowCut = monoChain.get<ChainPositions::LowCut>();
    auto& peakOne = monoChain.get<ChainPositions::PeakOne>();
    auto& peakTwo = monoChain.get<ChainPositions::PeakTwo>();
    auto& peakThree = monoChain.get<ChainPositions::PeakThree>();
    auto& highCut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> magnitudes;
    
    magnitudes.resize(responseWidth);
    
    for ( int i = 0; i < responseWidth; ++i )
    {
        double magnitude = 1.f;
        auto freq = mapToLog10(double(i) / double(responseWidth), 20.0, 20000.0);
    
        if (! monoChain.isBypassed<ChainPositions::PeakOne>() )
            magnitude *= peakOne.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! monoChain.isBypassed<ChainPositions::PeakTwo>() )
            magnitude *= peakTwo.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! monoChain.isBypassed<ChainPositions::PeakThree>() )
            magnitude *= peakThree.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (! lowCut.isBypassed<0>() )
            magnitude *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowCut.isBypassed<1>() )
            magnitude *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowCut.isBypassed<2>() )
            magnitude *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowCut.isBypassed<3>() )
            magnitude *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
    
        if (! highCut.isBypassed<0>() )
            magnitude *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highCut.isBypassed<1>() )
            magnitude *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highCut.isBypassed<2>() )
            magnitude *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highCut.isBypassed<3>() )
            magnitude *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        magnitudes[i] = Decibels::gainToDecibels(magnitude);
    }
    
    Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax] (double input)
    {
        return jmap(input, -24.5, 24.5, outputMin, outputMax);
    };
    
    
    responseCurve.startNewSubPath(responseArea.getX(), map(magnitudes.front()));
    
    for ( size_t i = 1; i < magnitudes.size(); ++i )
    {
        responseCurve.lineTo(responseArea.getX() + i,  map(magnitudes[i]));
    }
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}
//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
peakOneFreqSlider(*audioProcessor.apvts.getParameter("PeakOne Freq"), "Hz"),
peakOneGainSlider(*audioProcessor.apvts.getParameter("PeakOne Gain"), "dB"),
peakOneQualitySlider(*audioProcessor.apvts.getParameter("PeakOne Quality"), ""),
peakTwoFreqSlider(*audioProcessor.apvts.getParameter("PeakTwo Freq"), "Hz"),
peakTwoGainSlider(*audioProcessor.apvts.getParameter("PeakTwo Gain"), "dB"),
peakTwoQualitySlider(*audioProcessor.apvts.getParameter("PeakTwo Quality"), ""),
peakThreeFreqSlider(*audioProcessor.apvts.getParameter("PeakThree Freq"), "Hz"),
peakThreeGainSlider(*audioProcessor.apvts.getParameter("PeakThree Gain"), "dB"),
peakThreeQualitySlider(*audioProcessor.apvts.getParameter("PeakThree Quality"), ""),
lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), ""),
highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), ""),

responseCurveComponent(audioProcessor),
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
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.4);
    
    responseCurveComponent.setBounds(responseArea);
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.125);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.142857);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.45 ));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.45 ));
    highCutSlopeSlider.setBounds(highCutArea);
    
    auto peakOneArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto peakTwoArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);

    peakOneFreqSlider.setBounds(peakOneArea.removeFromTop(peakOneArea.getHeight() * 0.6 ));
    peakOneGainSlider.setBounds(peakOneArea.removeFromLeft(peakOneArea.getWidth() * 0.5 ));
    peakOneQualitySlider.setBounds(peakOneArea);

    peakTwoFreqSlider.setBounds(peakTwoArea.removeFromTop(peakTwoArea.getHeight() * 0.6 ));
    peakTwoGainSlider.setBounds(peakTwoArea.removeFromLeft(peakTwoArea.getWidth() * 0.5 ));
    peakTwoQualitySlider.setBounds(peakTwoArea);
    
    peakThreeFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.6 ));
    peakThreeGainSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.5 ));
    peakThreeQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &responseCurveComponent,
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
