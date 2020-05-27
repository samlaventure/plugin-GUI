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

#include "ClusterDecoderProcessorEditor.h"
#include "ClusterDecoderProcessor.h"
#include "ClusterDecoderProcessorVisualizer.h"
#include "../SleepScorer/SleepScorerFilter.h"
#include "../SleepScorer/SleepScorerProcessor.h"

static const int EDITOR_DESIRED_WIDTH = 270;


ClusterDecoderProcessorEditor::ClusterDecoderProcessorEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors = true)
    : VisualizerEditor (parentNode, useDefaultParameterEditors)
{
    tabText = "ClusterDecoder";
    lastFilePath = CoreServices::getDefaultUserSaveDirectory();

    // Open Ephys Plugin Generator will insert generated code for editor here. Don't edit this section.
    //[OPENEPHYS_EDITOR_PRE_CONSTRUCTOR_SECTION_BEGIN]

    //m_contentLookAndFeel = new LOOKANDFEELCLASSNAME();
    //content.setLookAndFeel (m_contentLookAndFeel);
    addAndMakeVisible (&content);
    content.toBack(); // to be able to see parameters components
    setDesiredWidth (EDITOR_DESIRED_WIDTH);

    //[OPENEPHYS_EDITOR_PRE_CONSTRUCTOR_SECTION_END]

    fileButton = new UtilityButton ("F:", Font ("Small Text", 13, Font::plain));
    fileButton->addListener (this);
    fileButton->setBounds (5, 27, 20, 20);
    addAndMakeVisible (fileButton);

    fileNameLabel = new Label ("FileNameLabel", "No file selected.");
    fileNameLabel->setBounds (30, 27, 140, 20);
    addAndMakeVisible (fileNameLabel);

    thresholdSlider = new Slider (Slider::SliderStyle::Rotary, Slider::TextEntryBoxPosition::TextBoxBelow);
    thresholdSlider->setBounds (170,25,75,75);
    thresholdSlider->addListener (this);
    thresholdSlider->setRange (0., 1., 0.01);
    thresholdSlider->setValue (0.);
    addAndMakeVisible (thresholdSlider);

    thresholdLabel = new Label ("ThresholdDescLabel", "probability threshold");
    thresholdLabel->setFont(thresholdLabel->getFont().withHeight(12));
    thresholdLabel->setBounds (170,90,75,50);
    addAndMakeVisible (thresholdLabel);

    sleepStateComboBox = new ComboBox("SleepStateComboBox");
    sleepStateComboBox->addListener(this);
    sleepStateComboBox->addItem("Any", SleepState::undefinedSleepState);
    sleepStateComboBox->addItem(SleepScorerProcessor::sleepStateAsText(SleepState::Wake), SleepState::Wake);
    sleepStateComboBox->addItem(SleepScorerProcessor::sleepStateAsText(SleepState::REM), SleepState::REM);
    sleepStateComboBox->addItem(SleepScorerProcessor::sleepStateAsText(SleepState::NREM), SleepState::NREM);
    sleepStateComboBox->addItem(SleepScorerProcessor::sleepStateAsText(SleepState::Sleep), SleepState::Sleep);
    sleepStateComboBox->setSelectedId(SleepState::undefinedSleepState);
    sleepStateComboBox->setBounds (74, 103, 71, 24);
    addAndMakeVisible (sleepStateComboBox);

}


ClusterDecoderProcessorEditor::~ClusterDecoderProcessorEditor()
{
}


void ClusterDecoderProcessorEditor::resized()
{
    // Don't edit this section.
    //[OPENEPHYS_EDITOR_PRE_RESIZE_SECTION_BEGIN]

    VisualizerEditor::resized();

    const int xPosInitial = 2;
    const int yPosIntiial = 23;
    const int contentWidth = (isMerger() || isSplitter() || isUtility())
                                    ? getWidth() - xPosInitial * 2
                                    : channelSelector->isVisible()
                                        ? channelSelector->getX() - xPosInitial * 2 - 5
                                        : drawerButton->getX() - xPosInitial * 2;
    content.setBounds (xPosInitial, yPosIntiial,
                       contentWidth, getHeight() - yPosIntiial - 7);

    //[OPENEPHYS_EDITOR_PRE_RESIZE_SECTION_END]
}

uint ClusterDecoderProcessorEditor::getSelectedGroup()
{
    return content.getSelectedGroup();
}
uint ClusterDecoderProcessorEditor::getSelectedUnit()
{
    return content.getSelectedUnit();
}

void ClusterDecoderProcessorEditor::setFile(String file)
{
    File fileToread(file);
    lastFilePath = fileToread.getParentDirectory();
    if (((ClusterDecoderProcessor*) getProcessor())->setFile(fileToread.getFullPathName())) {
        fileNameLabel->setText(fileToread.getFileName(), dontSendNotification);
    } else {
        clearEditor();
    }
    CoreServices::updateSignalChain(this);
    repaint();
}
void ClusterDecoderProcessorEditor::clearEditor()
{
    fileNameLabel->setText ("No file selected.", dontSendNotification);
    setGroupsAndUnitsOptions(std::vector<uint>());
}
void ClusterDecoderProcessorEditor::setGroupsAndUnitsOptions(std::vector<uint> nUnits)
{
    content.setGroupsAndUnits(nUnits);
}

/**
    The listener method that reacts to the button click. The same method is called for all buttons
    on the editor, so the button variable, which cointains a pointer to the button that called the method
    has to be checked to know which function to perform.
*/
void ClusterDecoderProcessorEditor::buttonEvent (Button* button)
{
    if (acquisitionIsActive) return;
    if (button == fileButton) {
        FileChooser chooseFileReaderFile ("Please select the file you want to load...", lastFilePath, "*.json");
        if (chooseFileReaderFile.browseForFileToOpen()) setFile(chooseFileReaderFile.getResult().getFullPathName());
    }
}

void ClusterDecoderProcessorEditor::comboBoxChanged(ComboBox* changedComboBox)
{
    if (changedComboBox == sleepStateComboBox) {
        ((ClusterDecoderProcessor*) getProcessor())->setSleepState((SleepState) sleepStateComboBox->getSelectedId());
    }
}

Visualizer* ClusterDecoderProcessorEditor::createNewCanvas()
{
    auto processor = dynamic_cast<ClusterDecoderProcessor*> (getProcessor());
    return new ClusterDecoderProcessorVisualizer (processor);
}


void ClusterDecoderProcessorEditor::startAcquisition()
{
    ((ClusterDecoderProcessor*) getProcessor())->startAcquisition();
    content.enable(false);
}
void ClusterDecoderProcessorEditor::stopAcquisition()
{
    content.enable(true);
}