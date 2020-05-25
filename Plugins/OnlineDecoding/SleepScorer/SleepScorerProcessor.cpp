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
#include <fftw3.h>

#include "SleepScorerFilter.h"
#include "SleepScorerProcessor.h"
#include "SleepScorerProcessorEditor.h"



SleepScorerProcessor::SleepScorerProcessor()
    : GenericProcessor ("Sleep Scorer"), sleepState(undefinedSleepState), scoringThread(filtersReady)

{
    setProcessorType (PROCESSOR_TYPE_FILTER);

    // Open Ephys Plugin Generator will insert generated code for parameters here. Don't edit this section.
    //[OPENEPHYS_PARAMETERS_SECTION_BEGIN]
    //[OPENEPHYS_PARAMETERS_SECTION_END]


    gammaAndRatioValues[0] = 0;
    gammaAndRatioValues[1] = 0;

    if (getNumInputs() > 0) {
        sampleRate = getDataChannel(0)->getSampleRate();
        gammaFilter = new SleepScorerFilter(getDataChannel(0)->getSampleRate(), Gamma);
        thetaFilter = new SleepScorerFilter(getDataChannel(0)->getSampleRate(), Theta);
        deltaFilter = new SleepScorerFilter(getDataChannel(0)->getSampleRate(), Delta);
        gammaFilter->setFilter();
        thetaFilter->setFilter();
        deltaFilter->setFilter();
        scoringThread.setFilters(gammaFilter, thetaFilter, deltaFilter);
        filtersReady = true;
    }
}


SleepScorerProcessor::~SleepScorerProcessor()
{
    fftw_cleanup();
}


/**
  If the processor uses a custom editor, this method must be present.
*/
AudioProcessorEditor* SleepScorerProcessor::createEditor()
{
    editor = new SleepScorerProcessorEditor (this, true);
    return editor;
}

void SleepScorerProcessor::createEventChannels()
{
    if (getNumInputs() > 0) {
        textChannel = new EventChannel(EventChannel::TEXT, 1, 20, getDataChannel(0)->getSampleRate(), this);

        textChannel->addMetaData(
            new MetaDataDescriptor(
                MetaDataDescriptor::CHAR,
                12,
                "ChannelName",
                "ChannelName",
                "sleepScoring.channelName"),
            new MetaDataValue(
                MetaDataDescriptor::CHAR,
                12,
                "SleepScorer\0"));

        textChannel->addEventMetaData(
            new MetaDataDescriptor(
                MetaDataDescriptor::FLOAT, 
                2, 
                "GammaAndRatio", 
                "gamma and theta/delta values", 
                "sleepScoring.values"));

        eventChannelArray.add(textChannel);
    }
}

void SleepScorerProcessor::updateSettings ()
{
    if (getNumInputs() > 0) {

        if (sampleRate != getDataChannel(0)->getSampleRate() and filtersReady) {
            filtersReady = false;
        }

        if (filtersReady) return;
        sampleRate = getDataChannel(0)->getSampleRate();
        gammaFilter = new SleepScorerFilter(getDataChannel(0)->getSampleRate(), Gamma);
        thetaFilter = new SleepScorerFilter(getDataChannel(0)->getSampleRate(), Theta);
        deltaFilter = new SleepScorerFilter(getDataChannel(0)->getSampleRate(), Delta);
        gammaFilter->setFilter();
        thetaFilter->setFilter();
        deltaFilter->setFilter();
        scoringThread.setFilters(gammaFilter, thetaFilter, deltaFilter);
        filtersReady = true;
    }
}

void SleepScorerProcessor::setParameter (int parameterIndex, float newValue)
{
    if (parameterIndex == 0) {
        std::cout << "setting gamma threshold to " << newValue << std::endl;
        gammaThreshold = newValue;
        return;
    }
    if (parameterIndex == 1) {
        std::cout << "setting theta threshold to " << newValue << std::endl;
        thetaThreshold = newValue;
        return;
    }
    if (parameterIndex == 2) {
        std::cout << "setting gamma Channel to " << newValue << std::endl;
        gammaChannel = newValue;
        return;
    }
    if (parameterIndex == 3) {
        std::cout << "setting theta Channel to " << newValue << std::endl;
        thetaChannel = newValue;
        return;
    }
}

const SleepState SleepScorerProcessor::sleepStateFromText(String newSleepState)
{
    if (newSleepState == "Wake") return Wake;
    else if (newSleepState == "REM") return REM;
    else if (newSleepState == "NREM") return NREM;
    else return undefinedSleepState;
}

const char* SleepScorerProcessor::sleepStateAsText(const SleepState newSleepState)
{
    switch(newSleepState) {
    case Wake: return "Wake";
    case REM: return "REM";
    case NREM: return "NREM";
    case undefinedSleepState: return "Undefined";
    default: return "Undefined";
    }
}

const char* SleepScorerProcessor::sleepStateAsText()
{
    return sleepStateAsText(sleepState);
}

void SleepScorerProcessor::computeSleepState()
{
    if (gammaAndRatioValues[0] > gammaThreshold) {
        sleepState = Wake;
    } else if (gammaAndRatioValues[1] > thetaThreshold) {
        sleepState = REM;
    } else {
        sleepState = NREM;
    }

    ((SleepScorerProcessorEditor*)getEditor())->editSleepScoringDisplay(sleepStateAsText());

    MetaDataValueArray md;
    md.add(new MetaDataValue(MetaDataDescriptor::FLOAT, 2, gammaAndRatioValues));
    TextEventPtr newText = TextEvent::createTextEvent(textChannel, lastSleepScoringTimestamp, sleepStateAsText(), md, 0);
    addEvent(textChannel, newText, 0);
}


void SleepScorerProcessor::process (AudioSampleBuffer& buffer)
{
    if (gammaFilter == nullptr or thetaFilter == nullptr or deltaFilter == nullptr) return;
    if (gammaFilter->isNewSleepScoringReady() and 
        thetaFilter->isNewSleepScoringReady() and
        deltaFilter->isNewSleepScoringReady()) {
        gammaAndRatioValues[0] = gammaFilter->rhythmPower();
        gammaAndRatioValues[1] = deltaFilter->rhythmPower();
        computeSleepState();
    }

    int numSamplesLeft = getNumSamples(gammaChannel);
    if (numSamplesLeft == 0) return;

    const float* gammaSamplePtr = buffer.getReadPointer(gammaChannel, 0);
    const float* thetaSamplePtr = buffer.getReadPointer(thetaChannel, 0);

    while (numSamplesLeft-- > 0) {
        newGammaSample += *(gammaSamplePtr++);
        newThetaSample += *(thetaSamplePtr++);
        
        if (++numOldSamplesInNewSample == DOWN_SAMPLING_FACTOR) {

            bool gammaReady = gammaFilter->addSampleAndCheckWindow(newGammaSample/DOWN_SAMPLING_FACTOR);
            bool thetaReady = thetaFilter->addSampleAndCheckWindow(newThetaSample/DOWN_SAMPLING_FACTOR);
            bool deltaReady = deltaFilter->addSampleAndCheckWindow(newThetaSample/DOWN_SAMPLING_FACTOR);

            if (gammaReady and thetaReady and deltaReady) {
                lastSleepScoringTimestamp = getTimestamp(gammaChannel) + getNumSamples(gammaChannel) - numSamplesLeft - 1;
            }
            numOldSamplesInNewSample = 0;
            newGammaSample = 0;
            newThetaSample = 0;
        }
    }
}



void SleepScorerProcessor::saveCustomParametersToXml (XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement ("SleepScorerProcessor");
    mainNode->setAttribute ("numParameters", getNumParameters());
    mainNode->setAttribute ("gammaThreshold", gammaThreshold);
    mainNode->setAttribute ("thetaThreshold", thetaThreshold);
    mainNode->setAttribute ("gammaChannel", gammaChannel);
    mainNode->setAttribute ("thetaChannel", thetaChannel);

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


void SleepScorerProcessor::loadCustomParametersFromXml()
{
    if (parametersAsXml == nullptr) // prevent double-loading
        return;

    // use parametersAsXml to restore state

    // Open Ephys Plugin Generator will insert generated code to load parameters here. Don't edit this section.
    //[OPENEPHYS_PARAMETERS_LOAD_SECTION_BEGIN]
    forEachXmlChildElement (*parametersAsXml, mainNode)
    {
        if (mainNode->hasTagName ("SleepScorerProcessor"))
        {
            int parameterIdx = -1;
            gammaThreshold = mainNode->getDoubleAttribute("gammaThreshold");
            thetaThreshold = mainNode->getDoubleAttribute("thetaThreshold");
            gammaChannel = mainNode->getIntAttribute("gammaChannel");
            thetaChannel = mainNode->getIntAttribute("thetaChannel");
            ((SleepScorerProcessorEditor*)getEditor())->editParametersDisplay(gammaChannel, thetaChannel, gammaThreshold, thetaThreshold);

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
