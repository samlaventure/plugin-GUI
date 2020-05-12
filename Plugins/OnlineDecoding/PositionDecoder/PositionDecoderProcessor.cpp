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
#include "PositionDecoderNetwork.h"
#include "PositionDecoderProcessor.h"
#include "PositionDecoderProcessorEditor.h"
#include "../SleepScorer/SleepScorerFilter.h"
#include "../SleepScorer/SleepScorerProcessor.h"


OnlineDecoding::Spike::Spike(int spkchn, int spl, int grp, int nChannels)
    : spikeChannel(spkchn),
    peakSample(spl),
    group(grp),
    ready(false)
{
    samples = std::vector<std::vector<float>> (nChannels, std::vector<float>());
}


PositionDecoderProcessor::PositionDecoderProcessor()
    : GenericProcessor ("Position Decoder"),
    sleepStateSelection(SleepState::undefinedSleepState),
    currentSleepState(SleepState::undefinedSleepState),
    nSamplesInCurrentWindow(0),
    output(nullptr)
{
    setProcessorType (PROCESSOR_TYPE_FILTER);
    // Open Ephys Plugin Generator will insert generated code for parameters here. Don't edit this section.
    //[OPENEPHYS_PARAMETERS_SECTION_BEGIN]
    //[OPENEPHYS_PARAMETERS_SECTION_END]
}


PositionDecoderProcessor::~PositionDecoderProcessor()
{
}


/**
  If the processor uses a custom editor, this method must be present.
*/
AudioProcessorEditor* PositionDecoderProcessor::createEditor()
{
    editor = new PositionDecoderProcessorEditor (this, true);
    return editor;
}


void PositionDecoderProcessor::setParameter (int parameterIndex, float newValue)
{
    GenericProcessor::setParameter (parameterIndex, newValue);
    editor->updateParameterButtons (parameterIndex);

}

void PositionDecoderProcessor::createEventChannels()
{
    if (getNumInputs() > 0) {
        ttlChannel = new EventChannel(EventChannel::TTL, 1, 1, getDataChannel(0)->getSampleRate(), this);

        ttlChannel->addEventMetaData(
            new MetaDataDescriptor(
                MetaDataDescriptor::FLOAT, 
                3, 
                "OnlineDecoding", 
                "position and std values", 
                "onlineDecoding.values"));

        eventChannelArray.add(ttlChannel);
        std::cout << "ttl channel created" << std::endl;
    }
}

bool PositionDecoderProcessor::setFile(String fullPath)
{
    jsonPath = fullPath;

    Json::Value jsonData;
    std::ifstream json_file(fullPath.toUTF8(), std::ifstream::binary);
    json_file >> jsonData;


    channels.resize(jsonData["nGroups"].asInt());

    for (uint group=0; group<channels.size(); ++group) {
        channels.at(group).resize( jsonData["group"+std::to_string(group)]["nChannels"].asInt() );

        for (uint chan=0; chan<channels.at(group).size(); ++chan) {
            channels.at(group).at(chan) = jsonData["group"+std::to_string(group)]["channel"+std::to_string(chan)].asInt();
        }
    }


    decoder = new PositionDecoderNetwork(jsonData["encodingPrefix"].asString(), channels);

    return true;
}

void PositionDecoderProcessor::setSleepState(SleepState newSleepState)
{
    sleepStateSelection = newSleepState;
    std::cout << "targetting sleep state: " << SleepScorerProcessor::sleepStateAsText(sleepStateSelection) << std::endl;
}

void PositionDecoderProcessor::startAcquisition()
{
    sampleRate = getDataChannel(channels.at(0).at(0))->getSampleRate();
}

void PositionDecoderProcessor::updateSettings()
{
    int nchannels = 0;
    for (uint group=0; group<channels.size(); ++group) nchannels += channels.at(group).size();
    if (getTotalSpikeChannels() == nchannels) {
        groups.resize(getTotalSpikeChannels());
        for (uint spikeChannel=0; spikeChannel<getTotalSpikeChannels(); ++spikeChannel) {

            int temp = 0, group = 0;
            while (spikeChannel >= temp + channels.at(group).size()) {
                ++group;
                temp += channels.at(group).size();
                if (group == channels.size()) {
                    group = -1;
                    break;
                }
            }

            groups.at(spikeChannel) = group;
        }
    } else {
        groups = std::vector<int>(getTotalSpikeChannels(),-1);
    }
}




bool PositionDecoderProcessor::checkOutput()
{
    PositionDecoderProcessorEditor* editor = (PositionDecoderProcessorEditor*) getEditor();

    if (output->X < editor->getXlowBound() or output->X > editor->getXhighBound()) return false;
    if (output->Y < editor->getYlowBound() or output->Y > editor->getYhighBound()) return false;
    if (output->stdDev < editor->getStdLowBound() or output->stdDev > editor->getStdHighBound()) return false;

    return true;
}


bool PositionDecoderProcessor::checkSleepState()
{
    if (sleepStateSelection == SleepState::undefinedSleepState) return true;
    else if (sleepStateSelection == currentSleepState and currentSleepStateLength > 1) return true;
    else return false;
}

void PositionDecoderProcessor::handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition)
{

    SpikeEventPtr newSpike = SpikeEvent::deserializeFromMessage(event, spikeInfo);
    spikesInWindow.add( 
        new OnlineDecoding::Spike(
            getSpikeChannelIndex(newSpike), 
            samplePosition, 
            groups.at(getSpikeChannelIndex(newSpike)),
            channels.at(groups.at(getSpikeChannelIndex(newSpike))).size()) );

    return;

}

void PositionDecoderProcessor::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition)
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

void PositionDecoderProcessor::process (AudioSampleBuffer& buffer)
{
    if (decoder == nullptr) return;
    checkForEvents(true);

    int channel, idx;
    const float* splPtr;
    std::vector<OnlineDecoding::Spike*> toDelete;

    // Copy full waveform
    for (auto& spk: spikesInWindow) {

        if (spk->ready) {
            continue;
        }

        if (signalBuffer.size() != buffer.getNumChannels() or spk->group == -1 or spk->peakSample == 0) {
            toDelete.push_back(spk);
            continue;
        }

        for (uint chan=0; chan<channels.at(spk->group).size(); ++chan) {

            idx = spk->peakSample - 15;
            channel = channels.at(spk->group).at(chan);

            while (idx < 0) spk->samples.at(chan).push_back(signalBuffer.getElement(channel, idx++));

            splPtr = buffer.getReadPointer(channel, idx);
            while (spk->samples.at(chan).size() < 32) {
                spk->samples.at(chan).push_back( *(splPtr++) );
            }
            spk->ready = true;
        }
    }

    // add spikes to decoder
    for (auto& spk: toDelete) spikesInWindow.removeObject(spk);
    for (auto& spk: spikesInWindow) {
        if (spk->ready) {
            if (nSamplesInCurrentWindow + spk->peakSample >= WINDOW_LENGTH * sampleRate) {
                decode();
                nSamplesInCurrentWindow -= WINDOW_LENGTH * sampleRate;
            }
            decoder->addSpike(spk->samples, spk->group);
            toDelete.push_back(spk);
        }
    }
    for (auto& spk: toDelete) spikesInWindow.removeObject(spk);


    // fill buffer in case of early spike in next window
    for (uint channel=0; channel<buffer.getNumChannels(); ++channel) {
        if (channel >= signalBuffer.size()) signalBuffer.push_back( std::vector<float>(signalBuffer.length, 0.) );

        for (uint spl=signalBuffer.length; spl>0; --spl) {
            signalBuffer(channel, signalBuffer.length-spl) = *(buffer.getReadPointer(channel, getNumSamples(channel)-spl));
        }
    }

    nSamplesInCurrentWindow += getNumSamples(channels.at(0).at(0));

    if (nSamplesInCurrentWindow >= WINDOW_LENGTH * sampleRate) {
        decode();
        spikesInWindow.clear();
        nSamplesInCurrentWindow -= WINDOW_LENGTH * sampleRate;
    }
}

void PositionDecoderProcessor::decode()
{
    output = decoder->inferPosition();

    MetaDataValueArray md;
    md.add(new MetaDataValue(MetaDataDescriptor::FLOAT, 3, output->getValues()));

    if (checkSleepState() and checkOutput()) {
        TTLEventPtr newTTL = TTLEvent::createTTLEvent(ttlChannel, getTimestamp(0), &ttlMessageUp, 1, md, 0);
        addEvent(ttlChannel, newTTL, 0);
    }
    TTLEventPtr newTTL2 = TTLEvent::createTTLEvent(ttlChannel, getTimestamp(0) + getNumSamples(0) - 1, &ttlMessageDown, 1, md, 0);
    addEvent(ttlChannel, newTTL2, getNumSamples(0) - 1);

    decoder->clearOutput();
}






void PositionDecoderProcessor::saveCustomParametersToXml (XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement ("PositionDecoderProcessor");
    mainNode->setAttribute ("numParameters", getNumParameters());
    mainNode->setAttribute ("path", jsonPath);

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


void PositionDecoderProcessor::loadCustomParametersFromXml()
{
    if (parametersAsXml == nullptr) // prevent double-loading
        return;

    // use parametersAsXml to restore state

    // Open Ephys Plugin Generator will insert generated code to load parameters here. Don't edit this section.
    //[OPENEPHYS_PARAMETERS_LOAD_SECTION_BEGIN]
    forEachXmlChildElement (*parametersAsXml, mainNode)
    {
        if (mainNode->hasTagName ("PositionDecoderProcessor"))
        {
            int parameterIdx = -1;
            String path = mainNode->getStringAttribute("path");
            if (path == "") std::cout << "no path." << std::endl;
            else ((PositionDecoderProcessorEditor*) getEditor())->setFile(path);

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
