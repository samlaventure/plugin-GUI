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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "SleepScorerProcessorContentComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SleepScorerProcessorContentComponent::SleepScorerProcessorContentComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    setLookAndFeel (defaultLookAndFeel);
    //[/Constructor_pre]

    addAndMakeVisible (gammaChannelEntry = new TextEditor ("gamma channel editor"));
    gammaChannelEntry->setMultiLine (false);
    gammaChannelEntry->setReturnKeyStartsNewLine (false);
    gammaChannelEntry->setReadOnly (false);
    gammaChannelEntry->setScrollbarsShown (true);
    gammaChannelEntry->setCaretVisible (true);
    gammaChannelEntry->setPopupMenuEnabled (true);
    gammaChannelEntry->setText (TRANS("0"));

    addAndMakeVisible (thetaChannelEntry = new TextEditor ("theta channel editor"));
    thetaChannelEntry->setMultiLine (false);
    thetaChannelEntry->setReturnKeyStartsNewLine (false);
    thetaChannelEntry->setReadOnly (false);
    thetaChannelEntry->setScrollbarsShown (true);
    thetaChannelEntry->setCaretVisible (true);
    thetaChannelEntry->setPopupMenuEnabled (true);
    thetaChannelEntry->setText (TRANS("0"));

    addAndMakeVisible (gammaThresholdEntry = new TextEditor ("gamma threshold editor"));
    gammaThresholdEntry->setMultiLine (false);
    gammaThresholdEntry->setReturnKeyStartsNewLine (false);
    gammaThresholdEntry->setReadOnly (false);
    gammaThresholdEntry->setScrollbarsShown (true);
    gammaThresholdEntry->setCaretVisible (true);
    gammaThresholdEntry->setPopupMenuEnabled (true);
    gammaThresholdEntry->setText (TRANS("0"));

    addAndMakeVisible (thetaThresholdEntry = new TextEditor ("theta threshold editor"));
    thetaThresholdEntry->setMultiLine (false);
    thetaThresholdEntry->setReturnKeyStartsNewLine (false);
    thetaThresholdEntry->setReadOnly (false);
    thetaThresholdEntry->setScrollbarsShown (true);
    thetaThresholdEntry->setCaretVisible (true);
    thetaThresholdEntry->setPopupMenuEnabled (true);
    thetaThresholdEntry->setText (TRANS("0"));

    addAndMakeVisible (gammaChannelText = new Label ("gamma channel",
                                                     TRANS("gamma channel :")));
    gammaChannelText->setFont (Font (15.00f, Font::plain));
    gammaChannelText->setJustificationType (Justification::centredLeft);
    gammaChannelText->setEditable (false, false, false);
    gammaChannelText->setColour (TextEditor::textColourId, Colours::black);
    gammaChannelText->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (gammaThresholdText = new Label ("gamma thresshold",
                                                       TRANS("gamma threshold :")));
    gammaThresholdText->setFont (Font (15.00f, Font::plain));
    gammaThresholdText->setJustificationType (Justification::centredLeft);
    gammaThresholdText->setEditable (false, false, false);
    gammaThresholdText->setColour (TextEditor::textColourId, Colours::black);
    gammaThresholdText->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (thetaChannelText = new Label ("theta chanel",
                                                     TRANS("theta channel :")));
    thetaChannelText->setFont (Font (15.00f, Font::plain));
    thetaChannelText->setJustificationType (Justification::centredLeft);
    thetaChannelText->setEditable (false, false, false);
    thetaChannelText->setColour (TextEditor::textColourId, Colours::black);
    thetaChannelText->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (thetaThresholdText = new Label ("theta threshold",
                                                       TRANS("theta threshold :")));
    thetaThresholdText->setFont (Font (15.00f, Font::plain));
    thetaThresholdText->setJustificationType (Justification::centredLeft);
    thetaThresholdText->setEditable (false, false, false);
    thetaThresholdText->setColour (TextEditor::textColourId, Colours::black);
    thetaThresholdText->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sleepstate = new Label ("sleep state",
                                               TRANS("Sleep state : ")));
    sleepstate->setFont (Font (15.00f, Font::plain));
    sleepstate->setJustificationType (Justification::centredLeft);
    sleepstate->setEditable (false, false, false);
    sleepstate->setColour (TextEditor::textColourId, Colours::black);
    sleepstate->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (150, 140);


    //[Constructor] You can add your own custom stuff here..
    gammaChannelEntry->setInputRestrictions(5, "0123456789");
    thetaChannelEntry->setInputRestrictions(5, "0123456789");
    gammaThresholdEntry->setInputRestrictions(5, "0123456789.");
    thetaThresholdEntry->setInputRestrictions(5, "0123456789.");
    sleepstate->setText("Undefined", dontSendNotification);
    //[/Constructor]
}

SleepScorerProcessorContentComponent::~SleepScorerProcessorContentComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    gammaChannelEntry = nullptr;
    thetaChannelEntry = nullptr;
    gammaThresholdEntry = nullptr;
    thetaThresholdEntry = nullptr;
    gammaChannelText = nullptr;
    gammaThresholdText = nullptr;
    thetaChannelText = nullptr;
    thetaThresholdText = nullptr;
    sleepstate = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SleepScorerProcessorContentComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffcfecf5));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SleepScorerProcessorContentComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    gammaChannelEntry->setBounds (80, 8, 55, 24);
    thetaChannelEntry->setBounds (80, 72, 55, 24);
    gammaThresholdEntry->setBounds (80, 40, 55, 24);
    thetaThresholdEntry->setBounds (80, 104, 55, 24);
    gammaChannelText->setBounds (8, 8, 72, 24);
    gammaThresholdText->setBounds (8, 40, 72, 24);
    thetaChannelText->setBounds (8, 72, 72, 24);
    thetaThresholdText->setBounds (8, 104, 72, 24);
    sleepstate->setBounds (1, 40, 150, 24);
    //[UserResized] Add your own custom resize handling here..
    int textWidth = 100;
    int x1 = 8, x2 = x1+textWidth, y1 = 8, y2 = 40;
    gammaChannelText->setBounds (x1, y1, textWidth, 24);
    gammaChannelEntry->setBounds (x2, y1, 55, 24);

    gammaThresholdText->setBounds (x1, y2, textWidth, 24);
    gammaThresholdEntry->setBounds (x2, y2, 55, 24);

    thetaChannelText->setBounds (200+x1, y1, textWidth, 24);
    thetaChannelEntry->setBounds (200+x2, y1, 55, 24);

    thetaThresholdText->setBounds (200+x1, y2, textWidth, 24);
    thetaThresholdEntry->setBounds (200+x2, y2, 55, 24);

    sleepstate->setBounds(8, 80, 150, 24);
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SleepScorerProcessorContentComponent::setSleepScoringDisplay(const String& newSleepState)
{
    sleepstate->setText(newSleepState, dontSendNotification);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SleepScorerProcessorContentComponent"
                 componentName="" parentClasses="public Component" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="150" initialHeight="140">
  <BACKGROUND backgroundColour="ffcfecf5"/>
  <TEXTEDITOR name="gamma channel editor" id="3ee973b4ecef17ca" memberName="gammaChannelEntry"
              virtualName="" explicitFocusOrder="0" pos="80 8 55 24" initialText="0"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="theta channel editor" id="3c0f65bb4dd790fb" memberName="thetaChannelEntry"
              virtualName="" explicitFocusOrder="0" pos="80 72 55 24" initialText="0"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="gamma threshold editor" id="cac5efe4e98a1911" memberName="gammaThresholdEntry"
              virtualName="" explicitFocusOrder="0" pos="80 40 55 24" initialText="0"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="theta threshold editor" id="d41b272efecf10" memberName="thetaThresholdEntry"
              virtualName="" explicitFocusOrder="0" pos="80 104 55 24" initialText="0"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <LABEL name="gamma channel" id="bc9146ed4094c7e7" memberName="gammaChannelText"
         virtualName="" explicitFocusOrder="0" pos="8 8 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="gamma channel :" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="gamma thresshold" id="8017b29a69ee29d7" memberName="gammaThresholdText"
         virtualName="" explicitFocusOrder="0" pos="8 40 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="gamma threshold :" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="theta chanel" id="ebbd898794e90315" memberName="thetaChannelText"
         virtualName="" explicitFocusOrder="0" pos="8 72 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="theta channel :" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="theta threshold" id="4339420fb436358f" memberName="thetaThresholdText"
         virtualName="" explicitFocusOrder="0" pos="8 104 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="theta threshold :" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="sleep state" id="ff70df1f868cd576" memberName="sleepstate"
         virtualName="" explicitFocusOrder="0" pos="1 40 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Sleep state : " editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
