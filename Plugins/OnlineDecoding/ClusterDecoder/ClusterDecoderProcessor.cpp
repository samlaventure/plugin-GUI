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
#include <fstream>

#include "../json.h"
#include "ClusterDecoderNetwork.h"
#include "ClusterDecoderProcessor.h"
#include "ClusterDecoderProcessorEditor.h"
#include "../SleepScorer/SleepScorerFilter.h"
#include "../SleepScorer/SleepScorerProcessor.h"


ClusterDecoderProcessor::ClusterDecoderProcessor()
    : GenericProcessor ("Cluster Decoder"),
    sleepStateSelection(SleepState::undefinedSleepState),
    currentSleepState(SleepState::undefinedSleepState)

{
    setProcessorType (PROCESSOR_TYPE_FILTER);
    // Open Ephys Plugin Generator will insert generated code for parameters here. Don't edit this section.
    //[OPENEPHYS_PARAMETERS_SECTION_BEGIN]
    //[OPENEPHYS_PARAMETERS_SECTION_END]
}


ClusterDecoderProcessor::~ClusterDecoderProcessor()
{
}


/**
  If the processor uses a custom editor, this method must be present.
*/
AudioProcessorEditor* ClusterDecoderProcessor::createEditor()
{
    editor = new ClusterDecoderProcessorEditor (this, true);
    return editor;
}


void ClusterDecoderProcessor::setParameter (int parameterIndex, float newValue)
{
    GenericProcessor::setParameter (parameterIndex, newValue);
    editor->updateParameterButtons (parameterIndex);

}

void ClusterDecoderProcessor::createEventChannels()
{
    if (getNumInputs() > 0) {
        ttlChannel = new EventChannel(EventChannel::TTL, 1, 1, getDataChannel(0)->getSampleRate(), this);
        eventChannelArray.add(ttlChannel);
        std::cout << "ttl channel created" << std::endl;
    }
}

bool ClusterDecoderProcessor::setFile(String fullPath)
{
    Json::Value jsonData;
    std::ifstream json_file(fullPath.toUTF8(), std::ifstream::binary);
    json_file >> jsonData;

    decoder = new ClusterDecoderNetwork(jsonData["encodingPrefix"].asString());

    std::vector<uint> nUnits;
    nUnits.resize(jsonData["nGroups"].asInt());
    nChannels.resize(jsonData["nGroups"].asInt());

    for (uint i=0; i<nUnits.size(); ++i) {
        nUnits[i] = jsonData["group"+std::to_string(i)]["nClusters"].asInt();
        nChannels[i] = jsonData["group"+std::to_string(i)]["nChannels"].asInt();
    }
    ((ClusterDecoderProcessorEditor*)getEditor())->setGroupsAndUnitsOptions(nUnits);

    return true;
}

void ClusterDecoderProcessor::setSleepState(SleepState newSleepState)
{
    sleepStateSelection = newSleepState;
    std::cout << "targetting sleep state: " << SleepScorerProcessor::sleepStateAsText(sleepStateSelection) << std::endl;
}

void ClusterDecoderProcessor::startAcquisition()
{
    if (decoder == nullptr) return;
    selectedGroup = ((ClusterDecoderProcessorEditor*)getEditor())->getSelectedGroup();
    selectedUnit  = ((ClusterDecoderProcessorEditor*)getEditor())->getSelectedUnit();
    decoder->setSpikeShape( nChannels.at(selectedGroup) );
}








bool ClusterDecoderProcessor::checkSleepState()
{
    if (sleepStateSelection == SleepState::undefinedSleepState) return true;
    else if (sleepStateSelection == SleepState::Sleep) {
        if (currentSleepState == SleepState::REM or currentSleepState == SleepState::NREM) {
            if (currentSleepStateLength > 1) return true;
            else return false;
        } else return false;
    }
    else if (sleepStateSelection == currentSleepState and currentSleepStateLength > 1) return true;
    else return false;
}

void ClusterDecoderProcessor::handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition)
{
    if (decoder==nullptr) return;
    SpikeEventPtr newSpike = SpikeEvent::deserializeFromMessage(event, spikeInfo);
    int clu = decoder->getCluster(newSpike, selectedGroup, ((ClusterDecoderProcessorEditor*)getEditor())->getThreshold());
    
    if (clu == selectedUnit and checkSleepState()) {
        TTLEventPtr newTTL = TTLEvent::createTTLEvent(ttlChannel, newSpike->getTimestamp(), &ttlMessageUp, 1, 0);
        addEvent(ttlChannel, newTTL, samplePosition);

        TTLEventPtr newTTL2 = TTLEvent::createTTLEvent(ttlChannel, newSpike->getTimestamp(), &ttlMessageDown, 1, 0);
        addEvent(ttlChannel, newTTL2, samplePosition);
    }
}

void ClusterDecoderProcessor::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition)
{
    if (Event::getEventType(event) == EventChannel::TEXT) {
        TextEventPtr text = TextEvent::deserializeFromMessage(event, eventInfo);

        String eventChannelName;
        text->getChannelInfo()->getMetaDataValue(0)->getValue(eventChannelName);
        if (eventChannelName != "SleepScorer") return;

        if (currentSleepState == SleepScorerProcessor::sleepStateFromText(text->getText())) {
            ++currentSleepStateLength;
        } else {
            currentSleepState = SleepScorerProcessor::sleepStateFromText(text->getText());
            currentSleepStateLength = 0;
        }
    }
}

void ClusterDecoderProcessor::process (AudioSampleBuffer& buffer)
{
    checkForEvents(true);
}









void ClusterDecoderProcessor::saveCustomParametersToXml (XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement ("ClusterDecoderProcessor");
    mainNode->setAttribute ("numParameters", getNumParameters());

    // Open Ephys Plugin Generator will insert generated code to save parameters here. Don't edit this section.
    //[OPENEPHYS_PARAMETERS_SAVE_SECTION_BEGIN]
    for (int i = 0; i < getNumParameters(); ++i)
    {
        XmlElement* parameterNode = mainNode->createNewChildElement ("Parameter");

        auto parameter = getParameterObject(i);
        parameterNode->setAttribute ("name", parameter->getName());
        parameterNode->setAttribute ("type", parameter->getParameterTypeString());

        auto parameterValue = getParameterVar (i, currentChannel);

        if (parameter->isBoolean())
            parameterNode->setAttribute ("value", (int)parameterValue);
        else if (parameter->isContinuous() || parameter->isDiscrete() || parameter->isNumerical())
            parameterNode->setAttribute ("value", (double)parameterValue);
    }
    //[OPENEPHYS_PARAMETERS_SAVE_SECTION_END]
}


void ClusterDecoderProcessor::loadCustomParametersFromXml()
{
    if (parametersAsXml == nullptr) // prevent double-loading
        return;

    // use parametersAsXml to restore state

    // Open Ephys Plugin Generator will insert generated code to load parameters here. Don't edit this section.
    //[OPENEPHYS_PARAMETERS_LOAD_SECTION_BEGIN]
    forEachXmlChildElement (*parametersAsXml, mainNode)
    {
        if (mainNode->hasTagName ("ClusterDecoderProcessor"))
        {
            int parameterIdx = -1;

            forEachXmlChildElement (*mainNode, parameterNode)
            {
                if (parameterNode->hasTagName ("Parameter"))
                {
                    ++parameterIdx;

                    String parameterType = parameterNode->getStringAttribute ("type");
                    if (parameterType == "Boolean")
                        setParameter (parameterIdx, parameterNode->getBoolAttribute ("value"));
                    else if (parameterType == "Continuous" || parameterType == "Numerical")
                        setParameter (parameterIdx, parameterNode->getDoubleAttribute ("value"));
                    else if (parameterType == "Discrete")
                        setParameter (parameterIdx, parameterNode->getIntAttribute ("value"));
                }
            }
        }
    }
    //[OPENEPHYS_PARAMETERS_LOAD_SECTION_END]
}
