/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __DEFAULTFILTEREDITOR__
#define __DEFAULTFILTEREDITOR__


#include <EditorHeaders.h>

class FilterViewport;

/**

  User interface for the DefaultFilterProcessor processor.

  @see DefaultFilterProcessor

*/

class DefaultFilterEditor : public GenericEditor,
    public Label::Listener
{
public:
    DefaultFilterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~DefaultFilterEditor();

    void buttonEvent(Button* button);
    void labelTextChanged(Label* label);

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);

    void setDefaults(double lowCut, double highCut);
    void resetToSavedText();

    void channelChanged (int chan, bool newState);

private:

    String lastHighCutString;
    String lastLowCutString;

    ScopedPointer<Label> highCutLabel;
    ScopedPointer<Label> lowCutLabel;

    ScopedPointer<Label> highCutValue;
    ScopedPointer<Label> lowCutValue;
    ScopedPointer<UtilityButton> applyFilterOnADC;
    ScopedPointer<UtilityButton> applyFilterOnChan;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DefaultFilterEditor);

};



#endif  // __DEFAULTFILTEREDITOR__
