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

#include <stdio.h>
#include "DefaultFilterProcessor.h"
#include "DefaultFilterEditor.h"
#include "DefaultFilter.h"


DefaultFilterProcessor::DefaultFilterProcessor()
    : GenericProcessor  ("Default Filter")
    , defaultLowCut     (350.0f)
    , defaultHighCut    (6000.0f)
{
    setProcessorType (PROCESSOR_TYPE_FILTER);
    applyOnADC = false;
}


DefaultFilterProcessor::~DefaultFilterProcessor()
{
}


AudioProcessorEditor* DefaultFilterProcessor::createEditor()
{
    editor = new DefaultFilterEditor (this, true);

    DefaultFilterEditor* ed = (DefaultFilterEditor*) getEditor();
    ed->setDefaults (defaultLowCut, defaultHighCut);

    return editor;
}



void DefaultFilterProcessor::updateSettings()
{
    //int id = nodeId;
    int numInputs = getNumInputs();
    // int numfilt = filtersOld.size();
    int numfilt = filters.size();
    if (numInputs < 1024 && numInputs != numfilt)
    {
        // SO fixed this. I think values were never restored correctly because you cleared lowCuts.
        Array<double> oldlowCuts;
        Array<double> oldhighCuts;
        oldlowCuts = lowCuts;
        oldhighCuts = highCuts;

        // filtersOld.clear();
        filters.clear();
        lowCuts.clear();
        highCuts.clear();
        shouldFilterChannel.clear();

        for (int n = 0; n < getNumInputs(); ++n)
        {
            filters.add(new DefaultFilter());
            // restore defaults

            shouldFilterChannel.add (true);

            float newLowCut  = 0.f;
            float newHighCut = 0.f;

            if (oldlowCuts.size() > n)
            {
                newLowCut  = oldlowCuts[n];
                newHighCut = oldhighCuts[n];
            }
            else
            {
                newLowCut  = defaultLowCut;
                newHighCut = defaultHighCut;
            }

            lowCuts.add  (newLowCut);
            highCuts.add (newHighCut);

            setFilterParameters (newLowCut, newHighCut, n);
        }
    }

    setApplyOnADC (applyOnADC);
}


double DefaultFilterProcessor::getLowCutValueForChannel (int chan) const
{
    return lowCuts[chan];
}


double DefaultFilterProcessor::getHighCutValueForChannel (int chan) const
{
    return highCuts[chan];
}


bool DefaultFilterProcessor::getBypassStatusForChannel (int chan) const
{
    return shouldFilterChannel[chan];
}


void DefaultFilterProcessor::setFilterParameters (double lowCut, double highCut, int chan)
{
    if (dataChannelArray.size() - 1 < chan)
        return;


    if (filters.size() > chan) {
        filters[chan]->setSampleFreq(dataChannelArray[chan]->getSampleRate());
        filters[chan]->setCutOffFreq(lowCut);
    }
}


void DefaultFilterProcessor::setParameter (int parameterIndex, float newValue)
{
    if (parameterIndex < 2) // change filter settings
    {
        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;

        if (parameterIndex == 0)
        {
            lowCuts.set (currentChannel,newValue);
        }
        else if (parameterIndex == 1)
        {
            highCuts.set (currentChannel,newValue);
        }

        setFilterParameters (lowCuts[currentChannel],
                             highCuts[currentChannel],
                             currentChannel);

        editor->updateParameterButtons (parameterIndex);
    }
    // change channel bypass state
    else
    {
        if (newValue == 0)
        {
            shouldFilterChannel.set (currentChannel, false);
        }
        else
        {
            shouldFilterChannel.set (currentChannel, true);
        }
    }
}


void DefaultFilterProcessor::process (AudioSampleBuffer& buffer)
{
    for (int n = 0; n < getNumOutputs(); ++n)
    {
        if (shouldFilterChannel[n])
        {
            float* ptr = buffer.getWritePointer (n);
            filters[n]->process(getNumSamples(n), ptr);
        }
    }
}


void DefaultFilterProcessor::setApplyOnADC (bool state)
{
    for (int n = 0; n < dataChannelArray.size(); ++n)
    {
        if (dataChannelArray[n]->getChannelType() == DataChannel::ADC_CHANNEL
            || dataChannelArray[n]->getChannelType() == DataChannel::AUX_CHANNEL)
        {
            setCurrentChannel (n);

            if (state)
                setParameter (2,1.0);
            else
                setParameter (2,0.0);
        }
    }
}


void DefaultFilterProcessor::saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelType)
{
    if (channelType == InfoObjectCommon::DATA_CHANNEL
        && channelNumber > -1
        && channelNumber < highCuts.size())
    {
        XmlElement* channelParams = channelInfo->createNewChildElement ("PARAMETERS");
        channelParams->setAttribute ("highcut",         highCuts[channelNumber]);
        channelParams->setAttribute ("lowcut",          lowCuts[channelNumber]);
        channelParams->setAttribute ("shouldFilter",    shouldFilterChannel[channelNumber]);
    }
}


void DefaultFilterProcessor::loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)
{
    int channelNum = channelInfo->getIntAttribute ("number");

    if (channelType == InfoObjectCommon::DATA_CHANNEL)
    {
        // restore high and low cut text in case they were changed by channelChanged
        static_cast<DefaultFilterEditor*>(getEditor())->resetToSavedText();

        forEachXmlChildElement (*channelInfo, subNode)
        {
            if (subNode->hasTagName ("PARAMETERS"))
            {
                highCuts.set (channelNum, subNode->getDoubleAttribute ("highcut", defaultHighCut));
                lowCuts.set  (channelNum, subNode->getDoubleAttribute ("lowcut",  defaultLowCut));
                shouldFilterChannel.set (channelNum, subNode->getBoolAttribute ("shouldFilter", true));

                setFilterParameters (lowCuts[channelNum], highCuts[channelNum], channelNum);
            }
        }
    }
}
