/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#ifndef __SPIKESORTEREDITORWITHSTIM_H_F0BD2DD9__
#define __SPIKESORTEREDITORWITHSTIM_H_F0BD2DD9__

#include <VisualizerEditorHeaders.h>

class SpikeSorterCanvasWithStim;

/**

  User interface for the SpikeSorter processor.

  Allows the user to add single electrodes, stereotrodes, or tetrodes.

  Parameters of individual channels, such as channel mapping, threshold,
  and enabled state, can be edited.

  @see SpikeSorter

*/

class SpikeSorterEditorWithStim : public VisualizerEditor,
    public Label::Listener,
    public ComboBox::Listener

{
public:
    SpikeSorterEditorWithStim(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~SpikeSorterEditorWithStim();
    void buttonEvent(Button* button);
    void labelTextChanged(Label* label);
    void comboBoxChanged(ComboBox* comboBox);
    void sliderEvent(Slider* slider);

    void channelChanged (int chan, bool newState) override;

    Visualizer* createNewCanvas();
    void checkSettings();
    void setThresholdValue(int chan, double threshold);
    OwnedArray<ElectrodeButton> electrodeButtons;
    SpikeSorterCanvasWithStim* spikeSorterCanvas;
    //void updateAdvancerList();
    void refreshElectrodeList(int selected = 0);
    void setSelectedElectrode(int i);
    int getSelectedElectrode();
    void setElectrodeComboBox(int direction);

private:
    void drawElectrodeButtons(int);
   



    //  ComboBox* electrodeTypes;
	ScopedPointer<ComboBox> electrodeList;// , dacCombo;
    ScopedPointer<ComboBox> advancerList;
    ScopedPointer<ComboBox> sleepStateList;
    ScopedPointer<Label> advancerLabel, depthOffsetLabel, depthOffsetEdit;
    ScopedPointer<Label> numElectrodes;
	ScopedPointer<Label> thresholdLabel; // , dacAssignmentLabel;
    ScopedPointer<TriangleButton> upButton;
    ScopedPointer<TriangleButton> downButton;
    ScopedPointer<UtilityButton> plusButton;
    ScopedPointer<UtilityButton> configButton;
    ScopedPointer<UtilityButton> removeElectrodeButton;
    ScopedPointer<UtilityButton> audioMonitorButton;
    ScopedPointer<ThresholdSlider> thresholdSlider;

    Array<String> advancerNames ;
    Array<int> advancerIDs;
    void removeElectrode(int index);
    void editElectrode(int index, int chan, int newChan);

    int lastId;
    bool isPlural;

    Font font;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeSorterEditorWithStim);

};




#endif  // __SPIKESORTEREDITORWITHSTIM_H_F0BD2DD9__
