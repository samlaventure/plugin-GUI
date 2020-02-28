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

#include "PositionDecoderProcessorContentComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
PositionDecoderProcessorContentComponent::PositionDecoderProcessorContentComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    setLookAndFeel (defaultLookAndFeel);
    //[/Constructor_pre]

    addAndMakeVisible (XtoText = new Label("new label", TRANS("to")));
    addAndMakeVisible (YtoText = new Label("new label", TRANS("to")));
    addAndMakeVisible (Xtext = new Label ("new label", TRANS("X bounds\n")));
    addAndMakeVisible (Ytext = new Label ("new label", TRANS("Y bounds\n")));
    addAndMakeVisible (StdText = new Label ("new label", TRANS("stand dev\n")));
    addAndMakeVisible (StdToText = new Label ("new label", TRANS("to\n")));
    setLabel(Xtext);
    setLabel(Ytext);
    setLabel(XtoText);
    setLabel(YtoText);
    setLabel(StdText);
    setLabel(StdToText);

    addAndMakeVisible (XlowBoundEditor = new TextEditor ("x low bound"));
    addAndMakeVisible (YlowBoundEditor = new TextEditor ("y low bound"));
    addAndMakeVisible (XhighBoundEditor = new TextEditor ("x high bound"));
    addAndMakeVisible (YhighBoundEditor = new TextEditor ("y high bound"));
    addAndMakeVisible (StdHighBoundEditor = new TextEditor ("std high bound"));
    addAndMakeVisible (StdLowBoundEditor = new TextEditor ("std low bound"));
    setTextEditor(XlowBoundEditor);
    setTextEditor(YlowBoundEditor);
    setTextEditor(XhighBoundEditor);
    setTextEditor(YhighBoundEditor);
    setTextEditor(StdHighBoundEditor);
    setTextEditor(StdLowBoundEditor);




    //[UserPreSize]
    //[/UserPreSize]

    setSize (150, 140);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

PositionDecoderProcessorContentComponent::~PositionDecoderProcessorContentComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    Xtext = nullptr;
    Ytext = nullptr;
    XtoText = nullptr;
    YtoText = nullptr;
    YlowBoundEditor = nullptr;
    XlowBoundEditor = nullptr;
    XhighBoundEditor = nullptr;
    YhighBoundEditor = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

void PositionDecoderProcessorContentComponent::setTextEditor(TextEditor* textEditor)
{
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (TRANS("0"));
    textEditor->setInputRestrictions(5, "0123456789.");
}
void PositionDecoderProcessorContentComponent::setLabel(Label* label)
{
    label->setFont (Font (15.10f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));
}

//==============================================================================
void PositionDecoderProcessorContentComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffcfecf5));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void PositionDecoderProcessorContentComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    Xtext->setBounds (proportionOfWidth (0.0533f), proportionOfHeight (0.2286f), 56, 32);
    Ytext->setBounds (proportionOfWidth (0.0533f), proportionOfHeight (0.5143f), 56, 32);
    YlowBoundEditor->setBounds (72, 72, 70, 24);
    XlowBoundEditor->setBounds (72, 32, 71, 24);
    //[UserResized] Add your own custom resize handling here..
    Xtext->setBounds (5, 25, 70, 32);
    Ytext->setBounds (5, 50, 70, 32);
    XtoText->setBounds (145, 25, 70, 32);
    YtoText->setBounds (145, 50, 70, 32);
    StdText->setBounds (5, 75, 70, 32);
    StdToText->setBounds (145, 75, 70, 32);
    XlowBoundEditor->setBounds (72, 30, 71, 24);
    YlowBoundEditor->setBounds (72, 55, 71, 24);
    XhighBoundEditor->setBounds (170, 30, 71, 24);
    YhighBoundEditor->setBounds (170, 55, 71, 24);
    StdLowBoundEditor->setBounds (72, 80, 71, 24);
    StdHighBoundEditor->setBounds (170, 80, 71, 24);
    //[/UserResized]
}




//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void PositionDecoderProcessorContentComponent::enable(bool enabled)
{
    XlowBoundEditor->setInterceptsMouseClicks(enabled, enabled);
    YlowBoundEditor->setInterceptsMouseClicks(enabled, enabled);
    XhighBoundEditor->setInterceptsMouseClicks(enabled, enabled);
    YhighBoundEditor->setInterceptsMouseClicks(enabled, enabled);
    StdLowBoundEditor->setInterceptsMouseClicks(enabled, enabled);
    StdHighBoundEditor->setInterceptsMouseClicks(enabled, enabled);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="PositionDecoderProcessorContentComponent"
                 componentName="" parentClasses="public Component" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="150" initialHeight="140">
  <BACKGROUND backgroundColour="ffcfecf5"/>
  <LABEL name="new label" id="c0213f1981531a0d" memberName="groupText"
         virtualName="" explicitFocusOrder="0" pos="5.333% 22.857% 56 32"
         edTextCol="ff000000" edBkgCol="0" labelText="group&#10;" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.099999999999999645" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="8675a1fd47da165e" memberName="clusterText"
         virtualName="" explicitFocusOrder="0" pos="5.333% 51.429% 56 32"
         edTextCol="ff000000" edBkgCol="0" labelText="unit&#10;" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.099999999999999645" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="new combo box" id="9bf5f6deb69028d9" memberName="clusterComboBox"
            virtualName="" explicitFocusOrder="0" pos="72 72 70 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <COMBOBOX name="new combo box" id="9c44a97736090e70" memberName="groupComboBox"
            virtualName="" explicitFocusOrder="0" pos="72 32 71 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
