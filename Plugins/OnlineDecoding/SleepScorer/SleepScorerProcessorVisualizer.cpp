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

#include "SleepScorerProcessorVisualizer.h"
#include "SleepScorerProcessor.h"


SleepScorerProcessorVisualizer::SleepScorerProcessorVisualizer (SleepScorerProcessor* procesor)
{
    // Open Ephys Plugin Generator will insert generated code for editor here. Don't edit this section.
    //[OPENEPHYS_EDITOR_PRE_CONSTRUCTOR_SECTION_BEGIN]

    //m_contentLookAndFeel = new LOOKANDFEELCLASSNAME();
    //content.setLookAndFeel (m_contentLookAndFeel);
    addAndMakeVisible (&content);

    //[OPENEPHYS_EDITOR_PRE_CONSTRUCTOR_SECTION_END]
}


SleepScorerProcessorVisualizer::~SleepScorerProcessorVisualizer()
{
}


void SleepScorerProcessorVisualizer::resized()
{
    content.setBounds (getLocalBounds());
}


void SleepScorerProcessorVisualizer::refreshState()
{
}


void SleepScorerProcessorVisualizer::update()
{
}


void SleepScorerProcessorVisualizer::refresh()
{
}


void SleepScorerProcessorVisualizer::beginAnimation()
{
    startCallbacks();
}


void SleepScorerProcessorVisualizer::endAnimation()
{
    stopCallbacks();
}


void SleepScorerProcessorVisualizer::setParameter (int parameter, float newValue)
{
}


void SleepScorerProcessorVisualizer::setParameter (int parameter, int val1, int val2, float newValue)
{
}
