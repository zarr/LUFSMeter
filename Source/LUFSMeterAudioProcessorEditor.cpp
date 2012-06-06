/*
 ===============================================================================
 
 LUFSMeterAudioProcessorEditor.cpp
 
 
 This file is part of the LUFS Meter audio measurement plugin.
 Copyright 2011-12 by Klangfreund, Samuel Gaehwiler.
 
 -------------------------------------------------------------------------------
 
 The LUFS Meter can be redistributed and/or modified under the terms of the GNU 
 General Public License Version 2, as published by the Free Software Foundation.
 A copy of the license is included with these source files. It can also be found
 at www.gnu.org/licenses.
 
 The LUFS Meter is distributed WITHOUT ANY WARRANTY.
 See the GNU General Public License for more details.
 
 -------------------------------------------------------------------------------
 
 To release a closed-source product which uses the LUFS Meter or parts of it,
 a commercial license is available. Visit www.klangfreund.com/lufsmeter for more
 information.
 
 ===============================================================================
 */


#include "LUFSMeterAudioProcessor.h"
#include "LUFSMeterAudioProcessorEditor.h"


//==============================================================================
LUFSMeterAudioProcessorEditor::LUFSMeterAudioProcessorEditor (LUFSMeterAudioProcessor* ownerFilter)
  : AudioProcessorEditor (ownerFilter),
    momentaryLoudnessValue (var(-300.0)),
    shortTermLoudnessValue (var(-300.0)),
    integratedLoudnessValue (var(-300.0)),
    distanceBetweenLevelBarAndTop (10),
    distanceBetweenLevelBarAndBottom (32),
    backgroundGridCaption (distanceBetweenLevelBarAndTop, distanceBetweenLevelBarAndBottom),
    momentaryLoudnessBar (momentaryLoudnessValue),
    shortTermLoudnessBar (shortTermLoudnessValue),
    integratedLoudnessBar (integratedLoudnessValue),
    momentaryLoudnessCaption (String::empty, "M"),
    shortTermLoudnessCaption (String::empty, "S"),
    integratedLoudnessCaption (String::empty, "I"),
    levelHistory (integratedLoudnessValue)
{
    // Add the background grid
    addAndMakeVisible (&backgroundGrid);
    
    // Add the background grid caption
    addAndMakeVisible (&backgroundGridCaption);
    
    // TEMP:
    // Add a label that will display the current timecode and status..
//    addAndMakeVisible (&infoLabel);
//    infoLabel.setColour (Label::textColourId, Colours::green);
    
    // Add the meter bars
    addAndMakeVisible (&momentaryLoudnessBar);    
    addAndMakeVisible (&shortTermLoudnessBar);
    addAndMakeVisible (&integratedLoudnessBar);
    
    // Add the numeric values
    addAndMakeVisible (&momentaryLoudnessNumeric);
    momentaryLoudnessNumeric.getLevelValueObject().referTo(momentaryLoudnessValue);
    addAndMakeVisible (&shortTermLoudnessNumeric);
    shortTermLoudnessNumeric.getLevelValueObject().referTo(shortTermLoudnessValue);
    addAndMakeVisible (&integratedLoudnessNumeric);
    integratedLoudnessNumeric.getLevelValueObject().referTo(integratedLoudnessValue);
    
    // Add the captions
    const int fontHeight = 16;
    const Font fontForCaptions (fontHeight);
    const Justification justification (Justification::horizontallyCentred);
    momentaryLoudnessCaption.setFont(fontForCaptions);
    momentaryLoudnessCaption.setColour (Label::textColourId, Colours::green);
    // momentaryLoudnessCaption.setColour (Label::backgroundColourId, Colours::red);
    momentaryLoudnessCaption.setJustificationType(justification);
    addAndMakeVisible (&momentaryLoudnessCaption);
    shortTermLoudnessCaption.setFont(fontForCaptions);
    shortTermLoudnessCaption.setColour (Label::textColourId, Colours::green);
    shortTermLoudnessCaption.setJustificationType(justification);
    addAndMakeVisible (&shortTermLoudnessCaption);
    integratedLoudnessCaption.setFont(fontForCaptions);
    integratedLoudnessCaption.setColour (Label::textColourId, Colours::green);
    integratedLoudnessCaption.setJustificationType(justification);
    addAndMakeVisible (&integratedLoudnessCaption);
    
    // Add the level history graph.
    addAndMakeVisible (&levelHistory);
    
    // Add the reset button
    resetButton.addListener(this);
    resetButton.setButtonText("reset");
    resetButton.setColour(TextButton::buttonColourId, Colours::green);
    addAndMakeVisible (&resetButton);

    
    // Add the triangular resizer component for the bottom-right of the UI.
    addAndMakeVisible (resizer = new ResizableCornerComponent (this, &resizeLimits));
    
    resizeLimits.setSizeLimits (150, 150, 800, 400);
    
    // Set our component's initial size to be the last one that was stored in 
    // the filter's settings.
    setSize (ownerFilter->lastUIWidth,
             ownerFilter->lastUIHeight);
    
    // Start the timer which will refresh the GUI elements.
    const int refreshIntervalInMilliseconds = 50;
    startTimer (refreshIntervalInMilliseconds);
}

LUFSMeterAudioProcessorEditor::~LUFSMeterAudioProcessorEditor()
{
//    momentaryLoudnessValue.removeListener(&momentaryLoudnessBar);
//    shortTermLoudnessValue.removeListener(&shortTermLoudnessBar);
//    integratedLoudnessValue.removeListener(&integratedLoudnessBar);
}

//==============================================================================
void LUFSMeterAudioProcessorEditor::paint (Graphics& g)
{
    // Draw the background
    g.fillAll(Colours::black);
}

void LUFSMeterAudioProcessorEditor::resized()
{
    // DBGT("Height of main component = " + String(getHeight()))
    
    const int levelBarBottomPosition = getHeight() - distanceBetweenLevelBarAndBottom;
    const int levelBarNumericTopPosition = getHeight() - distanceBetweenLevelBarAndBottom;
    const int levelBarCaptionTopPosition = getHeight() - 16;
    const int levelBarWidth = 50;
    const int backgroundGridCaptionWidth = 30;
    
    backgroundGrid.setBounds(0,
                             distanceBetweenLevelBarAndTop,
                             getWidth(),
                             levelBarBottomPosition - distanceBetweenLevelBarAndTop);
    
    backgroundGridCaption.setBounds(getWidth()-220, 0, backgroundGridCaptionWidth, getHeight());

    integratedLoudnessBar.setBounds(getWidth()-180, 
                                      distanceBetweenLevelBarAndTop, 
                                      levelBarWidth, 
                                      levelBarBottomPosition - distanceBetweenLevelBarAndTop);
    shortTermLoudnessBar.setBounds(getWidth()-120,
                                     distanceBetweenLevelBarAndTop,
                                     levelBarWidth,
                                     levelBarBottomPosition - distanceBetweenLevelBarAndTop);
    momentaryLoudnessBar.setBounds(getWidth()-60,
                                     distanceBetweenLevelBarAndTop,
                                     levelBarWidth,
                                     levelBarBottomPosition - distanceBetweenLevelBarAndTop);
    
    momentaryLoudnessNumeric.setBounds (getWidth()-60,
                                        levelBarNumericTopPosition,
                                        levelBarWidth,
                                        16);
    shortTermLoudnessNumeric.setBounds (getWidth()-120,
                                        levelBarNumericTopPosition,
                                        levelBarWidth,
                                        16);
    integratedLoudnessNumeric.setBounds (getWidth()-180,
                                         levelBarNumericTopPosition,
                                         levelBarWidth,
                                         16);
    
    momentaryLoudnessCaption.setBounds(getWidth()-60,
                                       levelBarCaptionTopPosition,
                                       levelBarWidth,
                                       16);
    shortTermLoudnessCaption.setBounds(getWidth()-120,
                                       levelBarCaptionTopPosition,
                                       levelBarWidth,
                                       16);
    integratedLoudnessCaption.setBounds(getWidth()-180,
                                       levelBarCaptionTopPosition,
                                       levelBarWidth,
                                       16);
    
    levelHistory.setBounds(0, distanceBetweenLevelBarAndTop, jmax(getWidth()-220, 0), getHeight()-distanceBetweenLevelBarAndTop);
    
//    const bool broadcastChangeMessage = false;
//    momentaryLoudnessCaption.setText("M", broadcastChangeMessage);

    // TEMP
    infoLabel.setBounds (10, 4, 380, 25);
    
//    resetButton.setBounds(10, getHeight()-35, 80, 25);
    resetButton.setBounds(10, 10, 80, 25);
    
    resizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);
    
    getProcessor()->lastUIWidth = getWidth();
    getProcessor()->lastUIHeight = getHeight();
}

LUFSMeterAudioProcessor* LUFSMeterAudioProcessorEditor::getProcessor() const
{
    return static_cast <LUFSMeterAudioProcessor*> (getAudioProcessor());
}

// quick-and-dirty function to format a timecode string
static const String timeToTimecodeString (const double seconds)
{
    const double absSecs = fabs (seconds);
    
    const int hours = (int) (absSecs / (60.0 * 60.0));
    const int mins  = ((int) (absSecs / 60.0)) % 60;
    const int secs  = ((int) absSecs) % 60;
    
    String s;
    if (seconds < 0)
        s = "-";
    
    s << String (hours).paddedLeft ('0', 2) << ":"
    << String (mins).paddedLeft ('0', 2) << ":"
    << String (secs).paddedLeft ('0', 2) << ":"
    << String (roundToInt (absSecs * 1000) % 1000).paddedLeft ('0', 3);
    
    return s;
}

// quick-and-dirty function to format a bars/beats string
static const String ppqToBarsBeatsString (double ppq, double /*lastBarPPQ*/, int numerator, int denominator)
{
    if (numerator == 0 || denominator == 0)
        return "1|1|0";
    
    const int ppqPerBar = (numerator * 4 / denominator);
    const double beats  = (fmod (ppq, ppqPerBar) / ppqPerBar) * numerator;
    
    const int bar    = ((int) ppq) / ppqPerBar + 1;
    const int beat   = ((int) beats) + 1;
    const int ticks  = ((int) (fmod (beats, 1.0) * 960.0));
    
    String s;
    s << bar << '|' << beat << '|' << ticks;
    return s;
}

// Updates the text in our position label.
void LUFSMeterAudioProcessorEditor::displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos)
{
    lastDisplayedPosition = pos;
    String displayText;
    displayText.preallocateBytes (128);
    
    displayText << String (pos.bpm, 2) << " bpm, "
    << pos.timeSigNumerator << '/' << pos.timeSigDenominator
    << "  -  " << timeToTimecodeString (pos.timeInSeconds);
//    << "  -  " << ppqToBarsBeatsString (pos.ppqPosition, pos.ppqPositionOfLastBarStart,
//                                        pos.timeSigNumerator, pos.timeSigDenominator);
    
    if (pos.isRecording)
        displayText << "  (recording)";
    else if (pos.isPlaying)
        displayText << "  (playing)";
    
    infoLabel.setText (displayText, false);
}

// This timer periodically updates the labels.
void LUFSMeterAudioProcessorEditor::timerCallback()
{
    LUFSMeterAudioProcessor* ourProcessor = getProcessor();
    
    AudioPlayHead::CurrentPositionInfo newPos (ourProcessor->lastPosInfo);
    
    if (lastDisplayedPosition != newPos)
        displayPositionInfo (newPos);
    
    float momentaryLoudnessOfFirstChannel = (ourProcessor->getMomentaryLoudness()).getFirst();
    jassert(momentaryLoudnessOfFirstChannel > -400)
    momentaryLoudnessValue.setValue(momentaryLoudnessOfFirstChannel);
    
    float shortTermLoudness = ourProcessor->getShortTermLoudness();
    jassert(shortTermLoudness > -400)
    shortTermLoudnessValue.setValue(shortTermLoudness);

    float integratedLoudness = ourProcessor->getIntegratedLoudness();
    jassert(integratedLoudness > -400)
    integratedLoudnessValue.setValue(integratedLoudness);
}

void LUFSMeterAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &resetButton)
    {
        getProcessor()->reset();
        levelHistory.reset();
    }
}