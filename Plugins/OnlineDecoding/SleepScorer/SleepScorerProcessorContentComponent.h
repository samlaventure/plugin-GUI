/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 4.2.1

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_636DD56E5CCC5C7A__
#define __JUCE_HEADER_636DD56E5CCC5C7A__

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class SleepScorerProcessorContentComponent  : public Component
{
public:
    //==============================================================================
    SleepScorerProcessorContentComponent ();
    ~SleepScorerProcessorContentComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void setSleepScoringDisplay(const String& newSleepState);
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    friend class SleepScorerProcessorEditor;
    SharedResourcePointer<LookAndFeel_V2> defaultLookAndFeel;
    ScopedPointer<Label> sleepStateText;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextEditor> gammaChannelEntry;
    ScopedPointer<TextEditor> thetaChannelEntry;
    ScopedPointer<TextEditor> gammaThresholdEntry;
    ScopedPointer<TextEditor> thetaThresholdEntry;
    ScopedPointer<Label> gammaChannelText;
    ScopedPointer<Label> gammaThresholdText;
    ScopedPointer<Label> thetaChannelText;
    ScopedPointer<Label> thetaThresholdText;
    ScopedPointer<Label> sleepstate;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SleepScorerProcessorContentComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_636DD56E5CCC5C7A__
