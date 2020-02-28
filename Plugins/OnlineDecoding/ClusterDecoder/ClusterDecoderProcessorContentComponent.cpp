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

#include "ClusterDecoderProcessorContentComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ClusterDecoderProcessorContentComponent::ClusterDecoderProcessorContentComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    setLookAndFeel (defaultLookAndFeel);
    //[/Constructor_pre]

    addAndMakeVisible (groupText = new Label ("new label",
                                              TRANS("group\n")));
    groupText->setFont (Font (15.10f, Font::plain));
    groupText->setJustificationType (Justification::centredLeft);
    groupText->setEditable (false, false, false);
    groupText->setColour (TextEditor::textColourId, Colours::black);
    groupText->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (clusterText = new Label ("new label",
                                                TRANS("unit\n")));
    clusterText->setFont (Font (15.10f, Font::plain));
    clusterText->setJustificationType (Justification::centredLeft);
    clusterText->setEditable (false, false, false);
    clusterText->setColour (TextEditor::textColourId, Colours::black);
    clusterText->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (clusterComboBox = new ComboBox ("new combo box"));
    clusterComboBox->setEditableText (false);
    clusterComboBox->setJustificationType (Justification::centredLeft);
    clusterComboBox->setTextWhenNothingSelected (String());
    clusterComboBox->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    clusterComboBox->addListener (this);

    addAndMakeVisible (groupComboBox = new ComboBox ("new combo box"));
    groupComboBox->setEditableText (false);
    groupComboBox->setJustificationType (Justification::centredLeft);
    groupComboBox->setTextWhenNothingSelected (String());
    groupComboBox->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    groupComboBox->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (150, 140);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ClusterDecoderProcessorContentComponent::~ClusterDecoderProcessorContentComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    groupText = nullptr;
    clusterText = nullptr;
    clusterComboBox = nullptr;
    groupComboBox = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ClusterDecoderProcessorContentComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffcfecf5));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ClusterDecoderProcessorContentComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    groupText->setBounds (proportionOfWidth (0.0533f), proportionOfHeight (0.2286f), 56, 32);
    clusterText->setBounds (proportionOfWidth (0.0533f), proportionOfHeight (0.5143f), 56, 32);
    clusterComboBox->setBounds (72, 72, 70, 24);
    groupComboBox->setBounds (72, 32, 71, 24);
    //[UserResized] Add your own custom resize handling here..
    clusterText->setBounds (32, 55, 56, 32);
    groupText->setBounds (25, 30, 56, 32);
    clusterComboBox->setBounds (72, 55, 71, 24);
    groupComboBox->setBounds (72, 30, 71, 24);
    //[/UserResized]
}

void ClusterDecoderProcessorContentComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == clusterComboBox)
    {
        //[UserComboBoxCode_clusterComboBox] -- add your combo box handling code here..
        //[/UserComboBoxCode_clusterComboBox]
    }
    else if (comboBoxThatHasChanged == groupComboBox)
    {
        //[UserComboBoxCode_groupComboBox] -- add your combo box handling code here..
        clusterComboBox->clear(dontSendNotification);
        uint selectedId = groupComboBox->getSelectedId();
        if (selectedId > 0) {
            for (uint i=0; i<nUnits.at(selectedId-1); ++i) {
                clusterComboBox->addItem(std::to_string(i).c_str(), i+1);
            }
        }
        clusterComboBox->setSelectedId(1, dontSendNotification);
        //[/UserComboBoxCode_groupComboBox]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
uint ClusterDecoderProcessorContentComponent::getSelectedGroup()
{
    return groupComboBox->getSelectedId()-1;
}
uint ClusterDecoderProcessorContentComponent::getSelectedUnit()
{
    return clusterComboBox->getSelectedId()-1;
}
void ClusterDecoderProcessorContentComponent::setGroupsAndUnits(std::vector<uint> nUnitsNew)
{
    nUnits = nUnitsNew;
    nGroups = nUnits.size();
    groupComboBox->clear(dontSendNotification);
    clusterComboBox->clear(dontSendNotification);
    for (uint i=0; i<nGroups; ++i) {
        groupComboBox->addItem(std::to_string(i).c_str(), i+1);          
    }
    groupComboBox->setSelectedId(1, sendNotification);
}
void ClusterDecoderProcessorContentComponent::enable(bool enabled)
{
    groupComboBox->setInterceptsMouseClicks(enabled, enabled);
    clusterComboBox->setInterceptsMouseClicks(enabled, enabled);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ClusterDecoderProcessorContentComponent"
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
