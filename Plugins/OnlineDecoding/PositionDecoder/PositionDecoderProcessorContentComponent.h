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

#ifndef __JUCE_HEADER_C198C4A6921FD97CPOSITION__
#define __JUCE_HEADER_C198C4A6921FD97CPOSITION__

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
class PositionDecoderProcessorContentComponent  : public Component
{
public:
    //==============================================================================
    PositionDecoderProcessorContentComponent ();
    ~PositionDecoderProcessorContentComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void enable(bool enabled);
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;

    static void setTextEditor(TextEditor *textEditor);
    static void setLabel(Label* label);

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    friend class PositionDecoderProcessorEditor;
    SharedResourcePointer<LookAndFeel_V2> defaultLookAndFeel;
    uint nGroups = 0;
    std::vector<uint> nUnits;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> Xtext, XtoText;
    ScopedPointer<Label> Ytext, YtoText;
    ScopedPointer<Label> StdText, StdToText;
    ScopedPointer<TextEditor> XlowBoundEditor, XhighBoundEditor;
    ScopedPointer<TextEditor> YlowBoundEditor, YhighBoundEditor;
    ScopedPointer<TextEditor> StdLowBoundEditor, StdHighBoundEditor;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PositionDecoderProcessorContentComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_C198C4A6921FD97CPOSITION__
