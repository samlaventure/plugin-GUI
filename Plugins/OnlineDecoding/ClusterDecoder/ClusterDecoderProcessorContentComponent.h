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

#ifndef __JUCE_HEADER_C198C4A6921FD97C__
#define __JUCE_HEADER_C198C4A6921FD97C__

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
class ClusterDecoderProcessorContentComponent  : public Component,
                                                 public ComboBoxListener
{
public:
    //==============================================================================
    ClusterDecoderProcessorContentComponent ();
    ~ClusterDecoderProcessorContentComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void setGroupsAndUnits(std::vector<uint> nUnitsNew);
    void enable(bool enabled);
    uint getSelectedGroup();
    uint getSelectedUnit();
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    friend class ClusterDecoderProcessorEditor;
    SharedResourcePointer<LookAndFeel_V2> defaultLookAndFeel;
    uint nGroups = 0;
    std::vector<uint> nUnits;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> groupText;
    ScopedPointer<Label> clusterText;
    ScopedPointer<ComboBox> clusterComboBox;
    ScopedPointer<ComboBox> groupComboBox;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClusterDecoderProcessorContentComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_C198C4A6921FD97C__
