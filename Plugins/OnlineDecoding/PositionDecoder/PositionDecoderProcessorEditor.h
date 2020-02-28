/*
   ------------------------------------------------------------------

   This file is part of the Open Ephys GUI
   Copyright (C) 2016 Open Ephys

   ------------------------------------------------------------------

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef POSITIONDECODERPROCESSOREDITOR_H_INCLUDED
#define POSITIONDECODERPROCESSOREDITOR_H_INCLUDED

#include <VisualizerEditorHeaders.h>
#include <AllLookAndFeels.h>
#include "PositionDecoderProcessorContentComponent.h"


/**
    This class serves as a template for creating new editors.

    If this were a real editor, this comment section would be used to
    describe the editor's structure. In this example, the editor will
    have a single button which will set a parameter in the processor.

    @see VisualizerEditor, GenericEditor
*/
class PositionDecoderProcessorEditor : 
  public VisualizerEditor, 
  public ComboBox::Listener, 
  public TextEditor::Listener
{
public:
    /** The class constructor, used to initialize any members. */
    PositionDecoderProcessorEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors);

    /** The class destructor, used to deallocate memory */
    ~PositionDecoderProcessorEditor();

    /**
        Unlike processors, which have a minimum set of required methods,
        editor are completely customized. There are still a couple of
        sometimes useful overloaded methods, which will appear here
    */

    /** This method executes each time editor is resized. */
    void resized() override;

    /** This method executes whenever a custom button is pressed */
    void setFile(String file);
    void buttonEvent (Button* button) override;
    void comboBoxChanged(ComboBox* changedComboBox) override;
    void textEditorReturnKeyPressed (TextEditor& text) override;
    void textEditorFocusLost (TextEditor& text) override {textEditorReturnKeyPressed(text);}

    /** Creates new canvas for visualizer editor. */
    Visualizer* createNewCanvas() override;

    /** Called to inform the editor that acquisition is about to start*/
    void startAcquisition();

    /** Called to inform the editor that acquisition has just stopped*/
    void stopAcquisition();

    /** Called whenever there is a change in the signal chain or it refreshes.
      It's called after the processors' same named method.
      */
    //void updateSettings();

    const float& getXlowBound() {return XlowBound;}
    const float& getXhighBound() {return XhighBound;}
    const float& getYlowBound() {return YlowBound;}
    const float& getYhighBound() {return YhighBound;}
    const float& getStdLowBound() {return StdLowBound;}
    const float& getStdHighBound() {return StdHighBound;}

private:
    // This component contains all components and graphics that were added using Projucer.
    // It's bounds initially the same bounds as the gray workspace (but only till the drawerButton for X)
    PositionDecoderProcessorContentComponent content;

    void clearEditor();
    File lastFilePath;
    ScopedPointer<UtilityButton> fileButton;
    ScopedPointer<Label> fileNameLabel;
    ScopedPointer<ComboBox> sleepStateComboBox;

    float XlowBound, XhighBound;
    float YlowBound, YhighBound;
    float StdLowBound, StdHighBound;

    void setXlowBound(float temp) {XlowBound = temp;}
    void setXhighBound(float temp) {XhighBound = temp;}
    void setYlowBound(float temp) {YlowBound = temp;}
    void setYhighBound(float temp) {YhighBound = temp;}
    void setStdLowBound(float temp) {StdLowBound = temp;}
    void setStdHighBound(float temp) {StdHighBound = temp;}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PositionDecoderProcessorEditor);
};

#endif // POSITIONDECODERPROCESSOREDITOR_H_INCLUDED
