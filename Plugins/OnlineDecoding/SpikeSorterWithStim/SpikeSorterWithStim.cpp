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

#include <stdio.h>
#include "SpikeSorterWithStim.h"
#include "SpikeSortBoxesWithStim.h"
#include "SpikeSorterCanvasWithStim.h"
#include "../OnlineDecoding/SleepScorer/SleepScorerFilter.h"
#include "../OnlineDecoding/SleepScorer/SleepScorerProcessor.h"

#define WINDOW_SIZE .04f

class spikeSorterWithStim;

SpikeSorterWithStim::SpikeSorterWithStim()
    : GenericProcessor("Spk Srt + Stim"),
      overflowBuffer(2,100), dataBuffer(nullptr),
      overflowBufferSize(100), currentElectrode(-1),
      numPreSamples(15),numPostSamples(17),
      sleepStateSelection(SleepState::undefinedSleepState),
      currentSleepState(SleepState::undefinedSleepState)
{
    setProcessorType (PROCESSOR_TYPE_FILTER);

    uniqueID = 0; // for electrode count
    uniqueSpikeID = 0;
    juce::Time timer;
    ticksPerSec = (float) timer.getHighResolutionTicksPerSecond();
    electrodeTypes.clear();
    electrodeCounter.clear();
    channelBuffers=nullptr;
    PCAbeforeBoxes = true;
    autoDACassignment = false;
    syncThresholds = false;
    flipSignal = false;
}

bool SpikeSorterWithStim::getFlipSignalState()
{
    return flipSignal;
}


void SpikeSorterWithStim::setFlipSignalState(bool state)
{
    flipSignal = state;

    mut.enter();
    if (currentElectrode >= 0)
    {
        if (electrodes[currentElectrode]->spikePlot != nullptr)
            electrodes[currentElectrode]->spikePlot->setFlipSignal(state);

    }
    mut.exit();

}

int SpikeSorterWithStim::getNumPreSamples()
{
    return numPreSamples;
}

int SpikeSorterWithStim::getNumPostSamples()
{
    return numPostSamples;
}

bool SpikeSorterWithStim::getAutoDacAssignmentStatus()
{
    return autoDACassignment;
}

bool SpikeSorterWithStim::getThresholdSyncStatus()
{
    return syncThresholds;
}

void SpikeSorterWithStim::setThresholdSyncStatus(bool status)
{
    syncThresholds= status;
}


void SpikeSorterWithStim::seteAutoDacAssignment(bool status)
{
    autoDACassignment = status;
}

void SpikeSorterWithStim::setNumPreSamples(int numSamples)
{
    // we need to update all electrodes, and also inform other modules that this has happened....
    numPreSamples = numSamples;

    for (int k = 0; k < electrodes.size(); k++)
    {
        electrodes[k]->resizeWaveform(numPreSamples,numPostSamples);
    }
}

void SpikeSorterWithStim::setNumPostSamples(int numSamples)
{
    numPostSamples = numSamples;
    for (int k = 0; k < electrodes.size(); k++)
    {
        electrodes[k]->resizeWaveform(numPreSamples,numPostSamples);
    }
}


int SpikeSorterWithStim::getUniqueProbeID(String type)
{
    for (int i = 0; i < electrodeTypes.size(); i++)
    {
        if (electrodeTypes[i] == type)
        {
            return electrodeCounter[i];
        }
    }
    // if we reached here, we didn't find the type. Add it.
    electrodeTypes.push_back(type);
    electrodeCounter.push_back(1);
    return 1;
}

void SpikeSorterWithStim::setEditAllState(bool val){
    editAll = val;
}

bool SpikeSorterWithStim::getEditAllState(){
    return editAll;
}

void SpikeSorterWithStim::increaseUniqueProbeID(String type)
{
    for (int i = 0; i < electrodeTypes.size(); i++)
    {
        if (electrodeTypes[i] == type)
        {
            electrodeCounter[i]++;
        }
    }
}



SpikeSorterWithStim::~SpikeSorterWithStim()
{
    if (channelBuffers != nullptr)
        delete channelBuffers;

}



AudioProcessorEditor* SpikeSorterWithStim::createEditor()
{
    editor = new SpikeSorterEditorWithStim(this, true);

    return editor;
}

void SpikeSorterWithStim::createEventChannels()
{
    if (getNumInputs() > 0) {
        std::cout<< "ttl channel created" <<std::endl;
        ttlChannel = new EventChannel(EventChannel::TTL, 1, 1, getDataChannel(0)->getSampleRate(), this);
        eventChannelArray.add(ttlChannel);
    }
}

void SpikeSorterWithStim::updateSettings()
{

    mut.enter();
	sorterReady = false;
    int numChannels = getNumInputs();
	if (numChannels > 0)
	{
		overflowBuffer.setSize(getNumInputs(), overflowBufferSize);
		overflowBuffer.clear();
	}

    if (channelBuffers != nullptr)
        delete channelBuffers;

    double SamplingRate = getSampleRate();;
    double ContinuousBufferLengthSec = 5;
    channelBuffers = new ContinuousCircularBufferWithStim(numChannels,SamplingRate,1, ContinuousBufferLengthSec);


    for (int i = 0; i < electrodes.size(); i++)
    {
		ElectrodeWithStim* elec = electrodes[i];
		unsigned int nChans = elec->numChannels;
		Array<const DataChannel*> chans;
		for (int c = 0; c < nChans; c++)
		{

			const DataChannel* ch = getDataChannel(elec->channels[c]);
			if (!ch)
			{
				//not enough channels for the electrodes
				return;
			}
			chans.add(ch);

		}

		SpikeChannel* spk = new SpikeChannel(SpikeChannel::typeFromNumChannels(nChans), this, chans);
		spk->setNumSamples(elec->prePeakSamples, elec->postPeakSamples);
		spk->addEventMetaData(new MetaDataDescriptor(MetaDataDescriptor::UINT8, 3, "Color", "Color of the spike", "graphics.color"));

        spikeChannelArray.add(spk);
    }
	sorterReady = true;
    mut.exit();

    if (getNumInputs() > 0) sampleRate = getDataChannel(0)->getSampleRate();
    for (auto el: electrodes) el->internalClock = -1;
}


ElectrodeWithStim::~ElectrodeWithStim()
{
    delete[] thresholds;
    delete[] isActive;
    delete[] voltageScale;
    delete[] channels;
    delete[] runningStats;
}

ElectrodeWithStim::ElectrodeWithStim(int ID, UniqueIDgeneratorWithStim* uniqueIDgenerator_, PCAcomputingThreadWithStim* pth, String _name, int _numChannels, int* _channels, float default_threshold, int pre, int post, float samplingRate , int sourceId, int subIdx)
{
    electrodeID = ID;
    computingThread = pth;
    uniqueIDgenerator = uniqueIDgenerator_;
    name = _name;
	sourceNodeId_ = sourceId;
	sourceSubIdx = subIdx;

    numChannels = abs(_numChannels);
    prePeakSamples = pre;
    postPeakSamples = post;

    thresholds = new double[numChannels];
    isActive = new bool[numChannels];
    channels = new int[numChannels];
    voltageScale = new double[numChannels];
    runningStats = new RunningStatWithStim[numChannels];
    depthOffsetMM = 0.0;

    advancerID = -1;

    for (int i = 0; i < numChannels; i++)
    {
        channels[i] = _channels[i];
        thresholds[i] = default_threshold;
        isActive[i] = true;
        voltageScale[i] = 500;
    }
    spikePlot = nullptr;

    if (computingThread != nullptr)
        spikeSort = new SpikeSortBoxesWithStim(uniqueIDgenerator, computingThread, numChannels, samplingRate, pre+post);
    else
        spikeSort = nullptr;

    isMonitored = false;

    internalClock = -1;
    isExcluded = _numChannels < 0;
}

void ElectrodeWithStim::resizeWaveform(int numPre, int numPost)
{
    // update electrode and all sorted units....
    // we can't keep pca space anymore, so we discard of all pca units (?)
    prePeakSamples = numPre;
    postPeakSamples = numPost;

    //spikePlot = nullptr;
    spikeSort->resizeWaveform(prePeakSamples+postPeakSamples);

}

void SpikeSorterWithStim::setElectrodeVoltageScale(int electrodeID, int index, float newvalue)
{
    std::vector<float> values;
    mut.enter();
    for (int k = 0; k < electrodes.size(); k++)
    {
        if (electrodes[k]->electrodeID == electrodeID)
        {
            electrodes[k]->voltageScale[index] = newvalue;
            mut.exit();
            return;
        }
    }
    mut.exit();
}

std::vector<float> SpikeSorterWithStim::getElectrodeVoltageScales(int electrodeID)
{
    std::vector<float> values;
    mut.enter();
    for (int k=0; k<electrodes.size(); k++)
    {
        if (electrodes[k]->electrodeID == electrodeID)
        {
            values.resize(electrodes[k]->numChannels);
            for (int i=0; i<electrodes[k]->numChannels; i++)
            {
                values[i] = electrodes[k]->voltageScale[i];
            }
            mut.exit();
            return values;
        }
    }
    mut.exit();
    return values;
}

// void SpikeSorterWithStim::setElectrodeAdvancerOffset(int i, double v)
// {
// 	mut.enter();
// 	if (i >= 0)
// 	{
// 		electrodes[i]->depthOffsetMM = v;
// 		addNetworkEventToQueue(StringTS("NewElectrodeDepthOffset "+String(electrodes[i]->electrodeID)+" "+String(v,4)));
// 	}
// 	mut.exit();
// }

// void SpikeSorterWithStim::setElectrodeAdvancer(int i, int ID)
// {
// 	mut.enter();
// 	if (i >= 0)
// 	{
// 		electrodes[i]->advancerID = ID;
// 	}
// 	mut.exit();
// }

void SpikeSorterWithStim::addNewUnit(int electrodeID, int newUnitID, uint8 r, uint8 g, uint8 b)
{
    String eventlog = "NewUnit "+String(electrodeID) + " "+String(newUnitID)+" "+String(r)+" "+String(g)+" "+String(b);
    //addNetworkEventToQueue(StringTS(eventlog));
  //  updateSinks(electrodeID,  newUnitID, r,g,b,true);
}

void SpikeSorterWithStim::removeUnit(int electrodeID, int unitID)
{
    String eventlog = "RemoveUnit "+String(electrodeID) + " "+String(unitID);
    //addNetworkEventToQueue(StringTS(eventlog));
 //   updateSinks(electrodeID,  unitID, 0,0,0,false);

}


void SpikeSorterWithStim::removeAllUnits(int electrodeID)
{
    String eventlog = "RemoveAllUnits "+String(electrodeID);
    //addNetworkEventToQueue(StringTS(eventlog));
 //   updateSinks(electrodeID,true);
}

/*RHD2000Thread* SpikeSorterWithStim::getRhythmAccess()
{

  / ProcessorGraph* gr = AccessClass::getProcessorGraph();
    Array<GenericProcessor*> p = gr->getListOfProcessors();
    for (int k=0; k<p.size(); k++)
    {
        if (p[k]->getName() == "Rhythm FPGA")
        {
            SourceNode* src = (SourceNode*)p[k];
            return (RHD2000Thread*)src->getThread();
        }
    }
    return nullptr;
}*/

void SpikeSorterWithStim::updateDACthreshold(int dacChannel, float threshold)
{
  /*  RHD2000Thread* th = getRhythmAccess();
    if (th != nullptr)
    {
        th->setDACthreshold(dacChannel,threshold);
    }*/
}

Array<int> SpikeSorterWithStim::getDACassignments()
{
   Array<int> dacChannels ;
 /*   RHD2000Thread* th = getRhythmAccess();
    if (th != nullptr)
    {
        dacChannels = th->getDACchannels();
    }*/
    return dacChannels;
}

int SpikeSorterWithStim::getDACassignment(int dacchannel)
{
 /*   RHD2000Thread* th = getRhythmAccess();
    if (th != nullptr)
    {
        Array<int> dacChannels = th->getDACchannels();
        return dacChannels[dacchannel];
    }
	*/
    return -1; // not assigned
}

void SpikeSorterWithStim::assignDACtoChannel(int dacOutput, int channel)
{
    // inform sinks about a new unit
    //getSourceNode()
/*    RHD2000Thread* th = getRhythmAccess();
    if (th != nullptr)
    {
        th->setDACchannel(dacOutput, channel); // this is probably wrong (JHS)
    }*/
}

void SpikeSorterWithStim::addElectrode(ElectrodeWithStim* newElectrode)
{
    mut.enter();
    resetElectrode(newElectrode);
    electrodes.add(newElectrode);
    // inform PSTH sink, if it exists, about this new electrode.
//    updateSinks(newElectrode);
    mut.exit();
}

bool SpikeSorterWithStim::addElectrode(int inChans, String name, double Depth)
{

    mut.enter();
    int firstChan;
    int nChans = abs(inChans);

    if (electrodes.size() == 0)
    {
        firstChan = 0;
    }
    else
    {
        ElectrodeWithStim* e = electrodes.getLast();
        firstChan = *(e->channels + (e->numChannels - 1)) + 1;
    }

    if (firstChan + nChans > getNumInputs())
    {
        mut.exit();
        return false;
    }

    int* chans = new int[nChans];
    for (int k = 0; k < nChans; k++)
        chans[k] = firstChan + k;

    ElectrodeWithStim* newElectrode = new ElectrodeWithStim(++uniqueID, &uniqueIDgenerator, &computingThread, name, inChans, chans, getDefaultThreshold(),
		numPreSamples, numPostSamples, getSampleRate(), dataChannelArray[chans[0]]->getSourceNodeID(), dataChannelArray[chans[0]]->getSubProcessorIdx());

    newElectrode->depthOffsetMM = Depth;
    String log = "Added electrode (ID "+ String(uniqueID)+") with " + String(nChans) + " channels." ;
    std::cout << log << std::endl;
    for (int i = 0; i < nChans; i++)
    {
        std::cout << "  Channel " << i << " = " << newElectrode->channels[i] << std::endl;
    }
    String eventlog = "NewElectrode "+ String(uniqueID) + " " + String(nChans) + " ";
    for (int k = 0; k < nChans; k++)
        eventlog += String(chans[k])+ " " + name;

    //addNetworkEventToQueue(StringTS(eventlog));
    delete[] chans;

    resetElectrode(newElectrode);
    electrodes.add(newElectrode);
 //   updateSinks(newElectrode);
    setCurrentElectrodeIndex(electrodes.size()-1);
    mut.exit();
    return true;

}

float SpikeSorterWithStim::getDefaultThreshold()
{
    return -20.0f;
}

StringArray SpikeSorterWithStim::getElectrodeNames()
{
    StringArray names;
    mut.enter();
    for (int i = 0; i < electrodes.size(); i++)
    {
        names.add(electrodes[i]->name);
    }
    mut.exit();
    return names;
}

void SpikeSorterWithStim::resetElectrode(ElectrodeWithStim* e)
{
    e->lastBufferIndex = 0;
}

bool SpikeSorterWithStim::removeElectrode(int index)
{
    mut.enter();
    // std::cout << "Spike detector removing electrode" << std::endl;

    if (index > electrodes.size() || index < 0)
    {
        mut.exit();
        return false;
    }


    String log = "Removing electrode (ID " + String(electrodes[index]->electrodeID)+")";
    std::cout << log <<std::endl;

    String eventlog = "RemoveElectrode " + String(electrodes[index]->electrodeID);
    //addNetworkEventToQueue(StringTS(eventlog));

    //int idToRemove = electrodes[index]->electrodeID;
    electrodes.remove(index);

    //(idToRemove);

    if (electrodes.size() > 0)
        currentElectrode = electrodes.size()-1;
    else
        currentElectrode = -1;

    mut.exit();
    return true;
}

void SpikeSorterWithStim::setElectrodeName(int index, String newName)
{
    mut.enter();
	if ((electrodes.size() > 0) && (index > 0))
	{
		electrodes[index - 1]->name = newName;
	//	updateSinks(electrodes[index - 1]->electrodeID, newName);
	}
    mut.exit();
}

void SpikeSorterWithStim::setChannel(int electrodeIndex, int channelNum, int newChannel)
{
    mut.enter();
    String log = "Setting electrode " + String(electrodeIndex) + " channel " + String(channelNum)+
                 " to " + String(newChannel);
    std::cout << log<< std::endl;



    String eventlog = "ChanelElectrodeChannel " + String(electrodes[electrodeIndex]->electrodeID) + " " + String(channelNum) + " " + String(newChannel);
    //addNetworkEventToQueue(StringTS(eventlog));

   // updateSinks(electrodes[electrodeIndex]->electrodeID, channelNum,newChannel);

    *(electrodes[electrodeIndex]->channels+channelNum) = newChannel;
    mut.exit();
}

int SpikeSorterWithStim::getNumChannels(int index)
{
    mut.enter();
    int i=electrodes[index]->numChannels;
    mut.exit();
    return i;
}

int SpikeSorterWithStim::getChannel(int index, int i)
{
    mut.enter();
    int ii=*(electrodes[index]->channels+i);
    mut.exit();
    return ii;
}


void SpikeSorterWithStim::setChannelActive(int electrodeIndex, int subChannel, bool active)
{

    currentElectrode = electrodeIndex;
    currentChannelIndex = subChannel;

    if (active)
        setParameter(98, 1);
    else
        setParameter(98, 0);

    //getEditorViewport()->makeEditorVisible(this, true, true);
}

bool SpikeSorterWithStim::isChannelActive(int electrodeIndex, int i)
{
    mut.enter();
    bool b= *(electrodes[electrodeIndex]->isActive+i);
    mut.exit();
    return b;
}


void SpikeSorterWithStim::setChannelThreshold(int electrodeNum, int channelNum, float thresh)
{
    mut.enter();
    currentElectrode = electrodeNum;
    currentChannelIndex = channelNum;
    electrodes[electrodeNum]->thresholds[channelNum] = thresh;
    if (electrodes[electrodeNum]->spikePlot != nullptr)
        electrodes[electrodeNum]->spikePlot->setDisplayThresholdForChannel(channelNum,thresh);

    if (syncThresholds)
    {
        for (int k=0; k<electrodes.size(); k++)
        {
            for (int i=0; i<electrodes[k]->numChannels; i++)
            {
                electrodes[k]->thresholds[i] = thresh;
            }
        }
    }

    mut.exit();
    setParameter(99, thresh);
}

double SpikeSorterWithStim::getChannelThreshold(int electrodeNum, int channelNum)
{
    mut.enter();
    double f= *(electrodes[electrodeNum]->thresholds+channelNum);
    mut.exit();
    return f;
}

void SpikeSorterWithStim::setParameter(int parameterIndex, float newValue)
{
    //editor->updateParameterButtons(parameterIndex);
    mut.enter();
    if (parameterIndex == 99 && currentElectrode > -1)
    {
        *(electrodes[currentElectrode]->thresholds+currentChannelIndex) = newValue;
    }
    else if (parameterIndex == 98 && currentElectrode > -1)
    {
        if (newValue == 0.0f)
            *(electrodes[currentElectrode]->isActive+currentChannelIndex) = false;
        else
            *(electrodes[currentElectrode]->isActive+currentChannelIndex) = true;
    }
    mut.exit();
}


bool SpikeSorterWithStim::enable()
{

    useOverflowBuffer.clear();
	if (!sorterReady)
	{
		CoreServices::sendStatusMessage("Not enough channels for the configured electrodes");
		std::cout << "SpikeSorterWithStim: Not enough channels for the configured electrodes" << std::endl;
		return false;
	}

    for (int i = 0; i < electrodes.size(); i++)
        useOverflowBuffer.add(false);


    SpikeSorterEditorWithStim* editor = (SpikeSorterEditorWithStim*) getEditor();
    editor->enable();

    for (auto el: electrodes) el->internalClock=-1;

    return true;
}



bool SpikeSorterWithStim::isReady()
{
    return true;
}


bool SpikeSorterWithStim::disable()
{
    mut.enter();
    for (int n = 0; n < electrodes.size(); n++)
    {
        resetElectrode(electrodes[n]);
    }
    //editor->disable();
    mut.exit();
    return true;
}

ElectrodeWithStim* SpikeSorterWithStim::getActiveElectrode()
{
    if (electrodes.size() == 0)
        return nullptr;

    return electrodes[currentElectrode];
}


void SpikeSorterWithStim::addWaveformToSpikeObject(SpikeEvent::SpikeBuffer& s,
                                           int& peakIndex,
                                           int& electrodeNumber,
                                           int& currentChannel)
{
    mut.enter();
	int spikeLength = electrodes[electrodeNumber]->prePeakSamples
		+ electrodes[electrodeNumber]->postPeakSamples;

	const int chan = *(electrodes[electrodeNumber]->channels + currentChannel);


	if (isChannelActive(electrodeNumber, currentChannel))
	{

		for (int sample = 0; sample < spikeLength; ++sample)
		{
			s.set(currentChannel, sample, getNextSample(*(electrodes[electrodeNumber]->channels + currentChannel)));
			++sampleIndex;

			//std::cout << currentIndex << std::endl;
		}
	}
	else
	{
		for (int sample = 0; sample < spikeLength; ++sample)
		{
			// insert a blank spike if the
			s.set(currentChannel, sample, 0);
			++sampleIndex;
			//std::cout << currentIndex << std::endl;
		}
	}

	sampleIndex -= spikeLength; // reset sample index
    mut.exit();

}

void SpikeSorterWithStim::startRecording()
{
    // send status messages about which electrodes and units are available.
    mut.enter();
    for (int k=0; k<electrodes.size(); k++)
    {
        String eventlog = "CurrentElectrodes "+String(electrodes[k]->electrodeID) + " "+ String(electrodes[k]->advancerID) + " "+String(electrodes[k]->depthOffsetMM) + " "+
                          String(electrodes[k]->numChannels) + " ";
        for (int i=0; i<electrodes[k]->numChannels; i++)
        {
            eventlog += String(electrodes[k]->channels[i])+ " " + electrodes[k]->name;
        }
        //addNetworkEventToQueue(StringTS(eventlog));

        std::vector<BoxUnitWithStim> boxUnits = electrodes[k]->spikeSort->getBoxUnits();
        for (int i=0; i<boxUnits.size(); i++)
        {
            String eventlog = "CurrentElectrodeUnits "+String(electrodes[k]->electrodeID) + " " + String(boxUnits[i].UnitID);
        }

        std::vector<PCAUnitWithStim> pcaUnits = electrodes[k]->spikeSort->getPCAUnits();
        for (int i=0; i<pcaUnits.size(); i++)
        {
            String eventlog = "CurrentElectrodeUnits "+String(electrodes[k]->electrodeID) + " " + String(pcaUnits[i].UnitID);
        }

    }

    mut.exit();
}
void SpikeSorterWithStim::stopRecording()
{
}

// int64 SpikeSorterWithStim::getExtrapolatedHardwareTimestamp(int64 softwareTS)
// {
// 	Time timer;
// 	// this is the case in which messages arrived before the data stream started....
// 	if (hardware_timestamp == 0)
// 		return 0;

// 	// compute how many ticks passed since the last known software-hardware pair
// 	int64 ticksPassed = software_timestamp-softwareTS;
// 	float secondPassed = (float)ticksPassed / timer.getHighResolutionTicksPerSecond();
// 	// adjust hardware stamp accordingly
// 	return hardware_timestamp + secondPassed*getSampleRate();
// }




// void SpikeSorterWithStim::postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events)
// {
// 	uint8* msg_with_ts = new uint8[s.len+8]; // for the two timestamps
// 	memcpy(msg_with_ts, s.str, s.len);
// 	memcpy(msg_with_ts+s.len, &s.timestamp, 8);

// 	addEvent(events,           // eventBuffer
//              (uint8) NETWORK,          // type
//              0,                // sampleNum
//              0,                // eventId
//              (uint8) GENERIC_EVENT,    // eventChannel
//              (uint8) s.len+8,  // numBytes
//              msg_with_ts);     // eventData

// 	delete msg_with_ts;
// }

// void SpikeSorterWithStim::addNetworkEventToQueue(StringTS S)
// {
// 	StringTS copy(S);
// 	getUIComponent()->getLogWindow()->addLineToLog(copy.getString());
// 	eventQueue.push(copy);
// }


// void SpikeSorterWithStim::postEventsInQueue(MidiBuffer& events)
// {
// 	while (eventQueue.size() > 0)
// 	{
// 		StringTS msg = eventQueue.front();
// 		postTimestamppedStringToMidiBuffer(msg,events);
// 		eventQueue.pop();
// 	}
// }


float SpikeSorterWithStim::getSelectedElectrodeNoise()
{
    if (electrodes.size() == 0)
        return 0.0;

    // TODO, change "0" to active channel to support tetrodes.
    return electrodes[currentElectrode]->runningStats[0].StandardDeviation();
}


void SpikeSorterWithStim::clearRunningStatForSelectedElectrode()
{
    if (electrodes.size() == 0)
        return;
    // TODO, change "0" to active channel to support tetrodes.
    electrodes[currentElectrode]->runningStats[0].Clear();
}

void SpikeSorterWithStim::setSleepState(const SleepState newSleepState)
{
    sleepStateSelection = newSleepState;
    std::cout << "targetting sleep state: " << SleepScorerProcessor::sleepStateAsText(sleepStateSelection) << std::endl;
}

bool SpikeSorterWithStim::checkSleepState()
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

void SpikeSorterWithStim::handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int sampleNum)
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

void SpikeSorterWithStim::process(AudioSampleBuffer& buffer)
{
    checkForEvents();

    //printf("Entering Spike Detector::process\n");
    mut.enter();
    uint16_t samplingFrequencyHz = getSampleRate();//buffer.getSamplingFrequency();
    // cycle through electrodes
    ElectrodeWithStim* electrode;
    dataBuffer = &buffer;
	const SpikeChannel* spikeChan;

    //channelBuffers->update(buffer, hardware_timestamp,software_timestamp, nSamples);

    for (int i = 0; i < electrodes.size(); i++)
    {

        //  std::cout << "ELECTRODE " << i << std::endl;

        electrode = electrodes[i];
		spikeChan = spikeChannelArray[i];

        // refresh buffer index for this electrode
        sampleIndex = electrode->lastBufferIndex - 1; // subtract 1 to account for
        // increment at start of getNextSample()

        int nSamples = getNumSamples(*electrode->channels); // get the number of samples for this buffer

        // cycle through samples
        while (samplesAvailable(nSamples))
        {

            int lastSample = sampleIndex++;

            // cycle through channels
            for (int chan = 0; chan < electrode->numChannels; chan++)
            {

                // std::cout << "  channel " << chan << std::endl;

                if (*(electrode->isActive+chan))
                {
                    //float v = getNextSample(currentChannel);

                    int currentChannel = electrode->channels[chan];
                    float currentValue = getNextSample(currentChannel);
                    electrode->runningStats[chan].Push(currentValue);

                    bool bSpikeDetectedPositive  = electrode->thresholds[chan] > 0 &&
                                                   (currentValue > electrode->thresholds[chan]); // rising edge
                    bool bSpikeDetectedNegative = electrode->thresholds[chan] < 0 &&
                                                  (currentValue < electrode->thresholds[chan]); // falling edge

                    if (bSpikeDetectedPositive || bSpikeDetectedNegative)
                    {

                        //std::cout << "Spike detected on electrode " << i << std::endl;
                        // find the peak
                        int peakIndex = sampleIndex;

                        //if (sampleIndex == 0 && i == 0)
                        //    std::cout << getCurrentSample(currentChannel) << std::endl;

                        if (bSpikeDetectedPositive)
                        {
                            // find localmaxima
                            while (getCurrentSample(currentChannel) < getNextSample(currentChannel) &&
                                   sampleIndex < peakIndex + electrode->postPeakSamples)
                            {
                                sampleIndex++;
                            }
                        }
                        else
                        {
                            // find local minimum

                            while (getCurrentSample(currentChannel) > getNextSample(currentChannel) &&
                                   sampleIndex < peakIndex + electrode->postPeakSamples)
                            {
                                sampleIndex++;
                            }
                        }

                        peakIndex = sampleIndex;
                        sampleIndex -= (electrode->prePeakSamples+1);

						const SpikeChannel* spikeChan = getSpikeChannel(i);
						SpikeEvent::SpikeBuffer spikeData(spikeChan);
						Array<float> thresholds;
						for (int channel = 0; channel < electrode->numChannels; ++channel)
						{
							addWaveformToSpikeObject(spikeData,
								peakIndex,
								i,
								channel);
							thresholds.add((int)*(electrode->thresholds + channel));
						}
						int64 timestamp = getTimestamp(electrode->channels[0]) + peakIndex;

						SorterSpikeWithStimPtr sorterSpike = new SorterSpikeContainerWithStim(spikeChan, spikeData, timestamp);

                        /*
                        bool perfectMatch = true;
                        for (int k=0;k<40;k++) {
                        	perfectMatch = perfectMatch & (prevSpike.data[k] == newSpike.data[k]);
                        }
                        if (perfectMatch)
                        {
                        	int x;
                        	x++;
                        }
                        */

                        //for (int xxx = 0; xxx < 1000; xxx++) // overload with spikes for testing purposes
						electrode->spikeSort->projectOnPrincipalComponents(sorterSpike);

                        // Add spike to drawing buffer....
						electrode->spikeSort->sortSpike(sorterSpike, PCAbeforeBoxes);


                        // transfer buffered spikes to spike plot
                        if (electrode->spikePlot != nullptr)
                        {
                            if (electrode->spikeSort->isPCAfinished())
                            {
                                electrode->spikeSort->resetJobStatus();
                                float p1min,p2min, p1max,  p2max;
                                electrode->spikeSort->getPCArange(p1min,p2min, p1max,  p2max);
                                electrode->spikePlot->setPCARange(p1min,p2min, p1max,  p2max);
                            }


							electrode->spikePlot->processSpikeObject(sorterSpike);
                        }

						MetaDataValueArray md;
						md.add(new MetaDataValue(MetaDataDescriptor::UINT8, 3, sorterSpike->color));
						SpikeEventPtr newSpike = SpikeEvent::createSpikeEvent(spikeChan, timestamp, thresholds, spikeData, sorterSpike->sortedId, md);
                        addSpike(spikeChan, newSpike, peakIndex);

                        // if (sorterSpike->sortedId > 0 and checkSleepState()) {
                        //     TTLEventPtr newTTL = TTLEvent::createTTLEvent(ttlChannel, timestamp, &ttlMessageUp, 1, 0);
                        //     addEvent(ttlChannel, newTTL, peakIndex);

                        //     TTLEventPtr newTTL2 = TTLEvent::createTTLEvent(ttlChannel, timestamp, &ttlMessageDown, 1, 0);
                        //     addEvent(ttlChannel, newTTL2, peakIndex);
                        // }
                        if (sorterSpike->sortedId > 0) electrode->atLeastOneRealSpike = true;

                        //prevSpike = newSpike;
                        // advance the sample index
                        sampleIndex = peakIndex + electrode->postPeakSamples;

                        break; // quit spike "for" loop
                    } // end spike trigger

                } // end if channel is active
            } // end cycle through channels on electrode

            

            electrode->internalClock += sampleIndex - lastSample;
            if ((float)electrode->internalClock/(float)sampleRate >= WINDOW_SIZE and electrode->windowComplete==false) {
                electrode->internalClock -= WINDOW_SIZE*getDataChannel(electrode->channels[0])->getSampleRate() + 1;
                electrode->windowComplete = true;
                electrode->lastWindowTimestamp = getTimestamp(electrode->channels[0]) + sampleIndex - electrode->internalClock - 1;
                electrode->spikeInWindow = electrode->atLeastOneRealSpike != electrode->isExcluded;
                electrode->atLeastOneRealSpike = false;
            }
        } // end cycle through samples

        //float vv = getNextSample(currentChannel);
        electrode->lastBufferIndex = sampleIndex - nSamples; // should be negative

        //jassert(electrode->lastBufferIndex < 0);

        if (nSamples > overflowBufferSize)
        {

            for (int j = 0; j < electrode->numChannels; j++)
            {
                //std::cout << "Processing " << *electrode->channels+i << std::endl;

                overflowBuffer.copyFrom(*(electrode->channels+j), 0,
                                        buffer, *(electrode->channels+j),
                                        nSamples-overflowBufferSize,
                                        overflowBufferSize);

            }

            useOverflowBuffer.set(i, true);

        }
        else
        {
            useOverflowBuffer.set(i, false);
        }

    } // end cycle through electrodes

    bool spikeOnAllElectrode = true;
    bool allWindowsComplete = true;
    for (auto el: electrodes) allWindowsComplete &= el->windowComplete;
    if (allWindowsComplete) {
        for (auto el: electrodes) {
            el->windowComplete = false;
            spikeOnAllElectrode &= el->spikeInWindow;
        }
    } else spikeOnAllElectrode = false;
    if (electrodes.size()>0 and spikeOnAllElectrode and checkSleepState()) sendTtlEvent(electrode->lastWindowTimestamp);

    mut.exit();
    //printf("Exitting Spike Detector::process\n");
}

void SpikeSorterWithStim::sendTtlEvent(int timestamp)
{
    TTLEventPtr newTTL = TTLEvent::createTTLEvent(ttlChannel, timestamp, &ttlMessageUp, 1, 0);
    addEvent(ttlChannel, newTTL, 0);

    TTLEventPtr newTTL2 = TTLEvent::createTTLEvent(ttlChannel, timestamp, &ttlMessageDown, 1, 0);
    addEvent(ttlChannel, newTTL2, 0);
}

float SpikeSorterWithStim::getNextSample(int& chan)
{



    //if (useOverflowBuffer)
    //{
    if (sampleIndex < 0)
    {
        // std::cout << "  sample index " << sampleIndex << "from overflowBuffer" << std::endl;
        int ind = overflowBufferSize + sampleIndex;

        if (ind < overflowBuffer.getNumSamples())
            return (*overflowBuffer.getReadPointer(chan, ind));
        else
            return 0;

    }
    else
    {
        //  useOverflowBuffer = false;
        // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;

        if (sampleIndex < getNumSamples(chan))
            return (*dataBuffer->getReadPointer(chan, sampleIndex));
        else
            return 0;
    }
    //} else {
    //    std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
    //     return *dataBuffer.getSampleData(chan, sampleIndex);
    //}

}

float SpikeSorterWithStim::getCurrentSample(int& chan)
{

    // if (useOverflowBuffer)
    // {
    //     return *overflowBuffer.getSampleData(chan, overflowBufferSize + sampleIndex - 1);
    // } else {
    //     return *dataBuffer.getSampleData(chan, sampleIndex - 1);
    // }

    if (sampleIndex < 1)
    {
        //std::cout << "  sample index " << sampleIndex << "from overflowBuffer" << std::endl;
        return (*overflowBuffer.getReadPointer(chan, overflowBufferSize + sampleIndex - 1)) ;
    }
    else
    {
        //  useOverflowBuffer = false;
        // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
        return (*dataBuffer->getReadPointer(chan, sampleIndex - 1));
    }
    //} else {

}


bool SpikeSorterWithStim::samplesAvailable(int nSamples)
{

    if (sampleIndex > nSamples - overflowBufferSize/2)
    {
        return false;
    }
    else
    {
        return true;
    }

}

void SpikeSorterWithStim::addProbes(String probeType,int numProbes, int nElectrodesPerProbe, int nChansPerElectrode,  double firstContactOffset, double interelectrodeDistance)
{
    for (int probeIter=0; probeIter<numProbes; probeIter++)
    {
        int probeCounter = getUniqueProbeID(probeType);
        for (int electrodeIter = 0; electrodeIter < nElectrodesPerProbe; electrodeIter++)
        {
            double depth = firstContactOffset - electrodeIter*interelectrodeDistance;
            String name;
            if (nElectrodesPerProbe > 1)
                name = probeType + " " + String(probeCounter) + " ["+String(electrodeIter+1)+"/"+String(nElectrodesPerProbe)+"]";
            else
                name = probeType + " " + String(probeCounter);

            bool successful = addElectrode(nChansPerElectrode, name,  depth);
            if (!successful)
            {
                CoreServices::sendStatusMessage("Not enough channels to add electrode.");
                return;
            }
        }
        increaseUniqueProbeID(probeType);
    }
}
const OwnedArray<ElectrodeWithStim>& SpikeSorterWithStim::getElectrodes()
{
    return electrodes;
}

// double SpikeSorterWithStim::getAdvancerPosition(int advancerID)
// {
// 	ProcessorGraph *g = getProcessorGraph();
// 	Array<GenericProcessor*> p = g->getListOfProcessors();
// 	for (int k=0;k<p.size();k++)
// 	{
// 		if (p[k] != nullptr)
// 		{
// 			if (p[k]->getName() == "Advancers")
// 			{
// 				AdvancerNode *node = (AdvancerNode*)p[k];
// 				return node->getAdvancerPosition(advancerID);
// 			}
// 		}
// 	}
// 	return 0.0;
// }

// double SpikeSorterWithStim::getElectrodeDepth(int electrodeID)
// {
// 	for (int k=0;k<electrodes.size();k++)
// 	{
// 		if (electrodes[k]->electrodeID == electrodeID)
// 		{
// 			double currentAdvancerPos = getAdvancerPosition(electrodes[k]->advancerID);
// 			return electrodes[k]->depthOffsetMM + currentAdvancerPos;
// 		}
// 	}
// 	return 0.0;
// }


// double SpikeSorterWithStim::getSelectedElectrodeDepth()
// {
// 	if (electrodes.size() == 0)
// 	return 0.0;

// 	double currentAdvancerPos = getAdvancerPosition(electrodes[currentElectrode]->advancerID);
// 	return electrodes[currentElectrode]->depthOffsetMM + currentAdvancerPos;
// }
void SpikeSorterWithStim::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("SpikeSorterWithStim");
    mainNode->setAttribute("numElectrodes", electrodes.size());

    SpikeSorterEditorWithStim* ed = (SpikeSorterEditorWithStim*) getEditor();

    mainNode->setAttribute("activeElectrode", ed->getSelectedElectrode()-1);
    mainNode->setAttribute("numPreSamples", numPreSamples);
    mainNode->setAttribute("numPostSamples", numPostSamples);
    mainNode->setAttribute("autoDACassignment",	autoDACassignment);
    mainNode->setAttribute("syncThresholds",syncThresholds);
    mainNode->setAttribute("uniqueID",uniqueID);
    mainNode->setAttribute("flipSignal",flipSignal);

    XmlElement* countNode = mainNode->createNewChildElement("ELECTRODE_COUNTER");

    countNode->setAttribute("numElectrodeTypes", (int)electrodeTypes.size());
    for (int k=0; k<electrodeTypes.size(); k++)
    {
        XmlElement* countNode2 = countNode->createNewChildElement("ELECTRODE_TYPE");
        countNode2->setAttribute("type", electrodeTypes[k]);
        countNode2->setAttribute("count", electrodeCounter[k]);
    }

    for (int i = 0; i < electrodes.size(); i++)
    {
        XmlElement* electrodeNode = mainNode->createNewChildElement("ELECTRODE");
        electrodeNode->setAttribute("name", electrodes[i]->name);
        electrodeNode->setAttribute("numChannels", electrodes[i]->numChannels);
        electrodeNode->setAttribute("prePeakSamples", electrodes[i]->prePeakSamples);
        electrodeNode->setAttribute("postPeakSamples", electrodes[i]->postPeakSamples);
        electrodeNode->setAttribute("advancerID", electrodes[i]->advancerID);
        electrodeNode->setAttribute("depthOffsetMM", electrodes[i]->depthOffsetMM);
        electrodeNode->setAttribute("electrodeID", electrodes[i]->electrodeID);

        for (int j = 0; j < electrodes[i]->numChannels; j++)
        {
            XmlElement* channelNode = electrodeNode->createNewChildElement("SUBCHANNEL");
            channelNode->setAttribute("ch",*(electrodes[i]->channels+j));
            channelNode->setAttribute("thresh",*(electrodes[i]->thresholds+j));
            channelNode->setAttribute("isActive",*(electrodes[i]->isActive+j));

        }

        // save spike sorting data.
        electrodes[i]->spikeSort->saveCustomParametersToXml(electrodeNode);

    }


}

void SpikeSorterWithStim::loadCustomParametersFromXml()
{

    if (parametersAsXml != nullptr)
    {

        int electrodeIndex = -1;

        forEachXmlChildElement(*parametersAsXml, mainNode)
        {

            // use parametersAsXml to restore state

            if (mainNode->hasTagName("SpikeSorterWithStim"))
            {
                //int numElectrodes = mainNode->getIntAttribute("numElectrodes");
                currentElectrode = mainNode->getIntAttribute("activeElectrode");
                numPreSamples = mainNode->getIntAttribute("numPreSamples");
                numPostSamples = mainNode->getIntAttribute("numPostSamples");
                autoDACassignment = mainNode->getBoolAttribute("autoDACassignment");
                syncThresholds = mainNode->getBoolAttribute("syncThresholds");
                uniqueID = mainNode->getIntAttribute("uniqueID");
                flipSignal = mainNode->getBoolAttribute("flipSignal");

                forEachXmlChildElement(*mainNode, xmlNode)
                {

                    if (xmlNode->hasTagName("ELECTRODE_COUNTER"))
                    {
                        int numElectrodeTypes = xmlNode->getIntAttribute("numElectrodeTypes");
                        electrodeCounter.resize(numElectrodeTypes);
                        electrodeTypes.resize(numElectrodeTypes);
                        int counter = 0;
                        forEachXmlChildElement(*xmlNode, xmltype)
                        {
                            if (xmltype->hasTagName("ELECTRODE_TYPE"))
                            {
                                electrodeTypes[counter] = xmltype->getStringAttribute("type");
                                electrodeCounter[counter] = xmltype->getIntAttribute("count");
                                counter++;
                            }
                        }
                    }
                    else if (xmlNode->hasTagName("ELECTRODE"))
                    {

                        electrodeIndex++;

                        int channelsPerElectrode = xmlNode->getIntAttribute("numChannels");

                        int advancerID = xmlNode->getIntAttribute("advancerID");
                        float depthOffsetMM = xmlNode->getDoubleAttribute("depthOffsetMM");
                        int electrodeID = xmlNode->getIntAttribute("electrodeID");
                        String electrodeName=xmlNode->getStringAttribute("name");


                        int channelIndex = -1;

                        int* channels = new int[channelsPerElectrode];
                        float* thres = new float[channelsPerElectrode];
                        bool* isActive = new bool[channelsPerElectrode];

                        forEachXmlChildElement(*xmlNode, channelNode)
                        {
                            if (channelNode->hasTagName("SUBCHANNEL"))
                            {
                                channelIndex++;
                                channels[channelIndex] = channelNode->getIntAttribute("ch");
                                thres[channelIndex] = channelNode->getDoubleAttribute("thresh");
                                isActive[channelIndex] = channelNode->getBoolAttribute("isActive");
                            }
                        }

                        int sourceNodeId = 102010; // some number

                        ElectrodeWithStim* newElectrode = new ElectrodeWithStim(electrodeID, &uniqueIDgenerator,&computingThread, electrodeName, channelsPerElectrode, channels,getDefaultThreshold(),
                                                                numPreSamples,numPostSamples, getSampleRate(), sourceNodeId,0);
                        for (int k=0; k<channelsPerElectrode; k++)
                        {
                            newElectrode->thresholds[k] = thres[k];
                            newElectrode->isActive[k] = isActive[k];
                        }
            			delete[] channels;
            			delete[] thres;
            			delete[] isActive;

                        newElectrode->advancerID = advancerID;
                        newElectrode->depthOffsetMM = depthOffsetMM;
                        // now read sorted units information
                        newElectrode->spikeSort->loadCustomParametersFromXml(xmlNode);
                        addElectrode(newElectrode);

                    }
                }
            }
        }
    }
    SpikeSorterEditorWithStim* ed = (SpikeSorterEditorWithStim*) getEditor();
    //	ed->updateAdvancerList();

    if (currentElectrode >= 0)
    {
        ed->refreshElectrodeList(currentElectrode);
        ed->setSelectedElectrode(1+currentElectrode);
    }
    else
    {
        ed->refreshElectrodeList();
    }


}



void SpikeSorterWithStim::removeSpikePlots()
{
    mut.enter();
    for (int i = 0; i < getNumElectrodes(); i++)
    {
        ElectrodeWithStim* ee = electrodes[i];
        ee->spikePlot = nullptr;
    }
    mut.exit();
}

int SpikeSorterWithStim::getNumElectrodes()
{
    mut.enter();
    int i= electrodes.size();
    mut.exit();
    return i;

}

int SpikeSorterWithStim::getNumberOfChannelsForElectrode(int i)
{
    mut.enter();
    if (i > -1 && i < electrodes.size())
    {
        ElectrodeWithStim* ee = electrodes[i];
        int ii=ee->numChannels;
        mut.exit();
        return ii;
    }
    else
    {
        mut.exit();
        return 0;
    }
}



String SpikeSorterWithStim::getNameForElectrode(int i)
{
    mut.enter();
    if (i > -1 && i < electrodes.size())
    {
        ElectrodeWithStim* ee = electrodes[i];
        String s= ee->name;
        mut.exit();
        return s;
    }
    else
    {
        mut.exit();
        return " ";
    }
}


void SpikeSorterWithStim::addSpikePlotForElectrode(SpikeHistogramPlotWithStim* sp, int i)
{
    mut.enter();
    ElectrodeWithStim* ee = electrodes[i];
    ee->spikePlot = sp;
    mut.exit();
}

int SpikeSorterWithStim::getCurrentElectrodeIndex()
{
    return currentElectrode;
}

ElectrodeWithStim* SpikeSorterWithStim::getElectrode(int i)
{
    return electrodes[i];
}


std::vector<int> SpikeSorterWithStim::getElectrodeChannels(int ID)
{
    std::vector<int> ch;
    mut.enter();
    for (int k=0; k<electrodes.size(); k++)
    {
        if (electrodes[k]->electrodeID == ID)
        {

            ch.resize(electrodes[k]->numChannels);
            for (int j=0; j<electrodes[k]->numChannels; j++)
            {
                ch[j] = electrodes[k]->channels[j];
            }

            mut.exit();
            return ch;
        }


    }
    mut.exit();
    return ch;
}

ElectrodeWithStim* SpikeSorterWithStim::setCurrentElectrodeIndex(int i)
{
    jassert(i >= 0 & i  < electrodes.size());
    currentElectrode = i;
    return electrodes[i];
}

/* Methods that try to communicate directly with PSTH. That is not allowed under plugin architecture, so this
must be rewritten using event channels to pass electrode data*/
#if 0
void SpikeSorterWithStim::updateSinks(int electrodeID, int unitID, uint8 r, uint8 g, uint8 b, bool addRemove)
{
    // inform sinks about a new unit
    ProcessorGraph* gr = AccessClass::getProcessorGraph();
    Array<GenericProcessor*> p = gr->getListOfProcessors();
    for (int k = 0; k<p.size(); k++)
    {
        if (p[k]->getName() == "PSTH")
        {
            PeriStimulusTimeHistogramNode* node = (PeriStimulusTimeHistogramNode*)p[k];
            if (node->trialCircularBuffer != nullptr)
            {
                if (addRemove)
                {
                    // add electrode
                    node->trialCircularBuffer->addNewUnit(electrodeID, unitID, r, g, b);
                }
                else
                {
                    // remove electrode
                    node->trialCircularBuffer->removeUnit(electrodeID, unitID);
                }
                ((PeriStimulusTimeHistogramEditor*)node->getEditor())->updateCanvas();
            }
        }
    }
}

void SpikeSorterWithStim::updateSinks(int electrodeID, bool rem)
{
    // inform sinks about a removal of all units
    ProcessorGraph* g = AccessClass::getProcessorGraph();
    Array<GenericProcessor*> p = g->getListOfProcessors();
    for (int k = 0; k<p.size(); k++)
    {
        if (p[k]->getName() == "PSTH")
        {
            PeriStimulusTimeHistogramNode* node = (PeriStimulusTimeHistogramNode*)p[k];
            if (node->trialCircularBuffer != nullptr)
            {
                if (rem)
                {
                    node->trialCircularBuffer->removeAllUnits(electrodeID);
                }
                (((PeriStimulusTimeHistogramEditor*)node->getEditor()))->updateCanvas();
            }
        }
        /*
        if (p[k]->getName() == "Spike Viewer")
        {
        	SpikeSorterWithStim* node = (SpikeSorterWithStim*)p[k];
        	node->syncWithSpikeSorter();
        }
        */
    }
}

void SpikeSorterWithStim::updateSinks(int electrodeID, int channelindex, int newchannel)
{
    // inform sinks about a channel change
    ProcessorGraph* g = AccessClass::getProcessorGraph();
    Array<GenericProcessor*> p = g->getListOfProcessors();
    for (int k = 0; k<p.size(); k++)
    {
        if (p[k]->getName() == "PSTH")
        {
            PeriStimulusTimeHistogramNode* node = (PeriStimulusTimeHistogramNode*)p[k];
            if (node->trialCircularBuffer != nullptr)
            {
                node->trialCircularBuffer->channelChange(electrodeID, channelindex, newchannel);
            }
        }

    }
}


void SpikeSorterWithStim::updateSinks(ElectrodeWithStim* electrode)
{
    // inform sinks about an electrode add
    ProcessorGraph* g = AccessClass::getProcessorGraph();
    Array<GenericProcessor*> p = g->getListOfProcessors();
    for (int k = 0; k<p.size(); k++)
    {
        String s = p[k]->getName();
        if (p[k]->getName() == "PSTH")
        {
            PeriStimulusTimeHistogramNode* node = (PeriStimulusTimeHistogramNode*)p[k];
            if (node->trialCircularBuffer != nullptr)
            {
                // add electrode
                node->trialCircularBuffer->addNewElectrode(electrode);
                (((PeriStimulusTimeHistogramEditor*)node->getEditor()))->updateCanvas();
            }
        }

    }
}

void SpikeSorterWithStim::updateSinks(int electrodeID, String NewName)
{
    // inform sinks about an electrode name change
    ProcessorGraph* g = AccessClass::getProcessorGraph();
    Array<GenericProcessor*> p = g->getListOfProcessors();
    for (int k = 0; k < p.size(); k++)
    {
        String s = p[k]->getName();
        if (p[k]->getName() == "PSTH")
        {
            PeriStimulusTimeHistogramNode* node = (PeriStimulusTimeHistogramNode*)p[k];
            if (node->trialCircularBuffer != nullptr)
            {
                // add electrode
                node->trialCircularBuffer->updateElectrodeName(electrodeID, NewName);
                (((PeriStimulusTimeHistogramEditor*)node->getEditor()))->updateCanvas();
            }
        }

    }
}


void SpikeSorterWithStim::updateSinks(int electrodeID)
{
    // inform sinks about an electrode removal
    ProcessorGraph* g = AccessClass::getProcessorGraph();
    Array<GenericProcessor*> p = g->getListOfProcessors();
    for (int k = 0; k<p.size(); k++)
    {
        String s = p[k]->getName();
        if (p[k]->getName() == "PSTH")
        {
            PeriStimulusTimeHistogramNode* node = (PeriStimulusTimeHistogramNode*)p[k];
            if (node->trialCircularBuffer != nullptr)
            {
                // remove electrode
                node->trialCircularBuffer->removeElectrode(electrodeID);
                ((PeriStimulusTimeHistogramEditor*)node->getEditor())->updateCanvas();
            }
        }

    }
}
#endif
/*


Histogram::Histogram(float _minValue, float _maxValue, float _resolution, bool _throwOutsideSamples) :
	minValue(_minValue), maxValue(_maxValue), resolution(_resolution), throwOutsideSamples(_throwOutsideSamples)
{
	numBins = 1+ abs(maxValue-minValue) / resolution;
	binCounts = new unsigned long[numBins];
	binCenters = new float[numBins];
	float deno = (numBins-1)/abs(maxValue-minValue);
	for (int k=0;k<numBins;k++)
	{
		binCounts[k] = 0;
		binCenters[k] = minValue + k/deno;
	}
}
//
//Histogram::Histogram(float _minValue, float _maxValue, int _numBins, bool _throwOutsideSamples) :
//	minValue(_minValue), maxValue(_maxValue), numBins(_numBins), throwOutsideSamples(_throwOutsideSamples)
//{
//	resolution = abs(maxValue-minValue) / numBins ;
//	binCounts = new int[numBins];
//	binCenters = new float[numBins];
//	for (int k=0;k<numBins;k++)
//	{
//		binCounts[k] = 0;
//		binCenters[k] = minValue + k/(numBins-1)*resolution;
//	}
//
//}

void Histogram::clear()
{
for (int k=0;k<numBins;k++)
	{
		binCounts[k] = 0;
	}
}


void Histogram::addSamples(float *Samples, int numSamples) {
	for (int k=0;k<numSamples;k++)
	{
		int indx = ceil( (Samples[k] - minValue) / (maxValue-minValue) * (numBins-1));
		if (indx >= 0 && indx < numBins)
			binCounts[indx]++;
	}
}

Histogram::~Histogram()
{
		delete [] binCounts;
		delete [] binCenters;
}

	*/













/***********************************/
//
//circularBuffer::circularBuffer(int NumCh, int NumSamplesToHoldPerChannel, double SamplingRate)
//{
//            numCh = NumCh;
//            samplingRate = SamplingRate;
//            Buf.resize(numCh);
//			for (int ch=0;ch<numCh;ch++) {
//				Buf[ch].resize(NumSamplesToHoldPerChannel);
//			}
//            BufTS_H.resize(NumSamplesToHoldPerChannel);
//            BufTS_S.resize(NumSamplesToHoldPerChannel);
//            bufLen = NumSamplesToHoldPerChannel;
//            numSamplesInBuf = 0;
//            ptr = 0; // points to a valid position in the buffer.
//}
//
//circularBuffer::~circularBuffer()
//{
//
//}
//
//
//std::vector<double> circularBuffer::getDataArray(int channel, int N)
//{
//	std::vector<double> LongArray;
//	LongArray.resize(N);
//	mut.enter();
//
//            int p = ptr - 1;
//            for (int k = 0; k < N; k++)
//            {
//                if (p < 0)
//                    p = bufLen - 1;
//                LongArray[k] = Buf[channel][p];
//                p--;
//            }
//            mut.exit();
//            return LongArray;
//}
//
//void circularBuffer::addDataToBuffer(std::vector<std::vector<double>> Data, double SoftwareTS, double HardwareTS)
//{
//	mut.enter();
//	int iNumPoints = Data[0].size();
//	for (int k = 0; k < iNumPoints; k++)
//	{
//		BufTS_H[ptr] = HardwareTS + k;
//		BufTS_S[ptr] = SoftwareTS + k / samplingRate;
//		for (int ch = 0; ch < numCh; ch++)
//		{
//			Buf[ch, ptr] = Data[ch, k];
//		}
//		ptr++;
//
//		if (ptr == bufLen)
//		{
//			ptr = 0;
//		}
//		numSamplesInBuf++;
//		if (numSamplesInBuf >= bufLen)
//		{
//			numSamplesInBuf = bufLen;
//		}
//	}
//	mut.exit();
//}
//
//
//double circularBuffer::findThresholdForChannel(int channel)
//{
//	// Run median on analog input
//	double numSamplesPerSecond = 30000;
//	std::vector<double> LongArray = getDataArray(channel, numSamplesPerSecond*5);
//
//	for (int k = 0; k < LongArray.size(); k++)
//		LongArray[k] = fabs(LongArray[k]);
//
//	std::sort (LongArray.begin(), LongArray.begin()+LongArray.size());           //(12 32 45 71)26 80 53 33
//
//
//	int Middle = LongArray.size() / 2;
//	double Median = LongArray[Middle];
//	double NewThres = -4.0F * Median / 0.675F;
//
//	return NewThres;
//}


// ===================================================

void ContinuousCircularBufferWithStim::reallocate(int NumCh)
{
    numCh =NumCh;
    Buf.resize(numCh);
    for (int k=0; k< numCh; k++)
    {
        Buf[k].resize(bufLen);
    }
    numSamplesInBuf = 0;
    ptr = 0; // points to a valid position in the buffer.

}


ContinuousCircularBufferWithStim::ContinuousCircularBufferWithStim(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer)
{
    Time t;

    numTicksPerSecond = (double) t.getHighResolutionTicksPerSecond();

    int numSamplesToHoldPerChannel = (int)(SamplingRate * NumSecInBuffer / SubSampling);
    buffer_dx = 1.0 / (SamplingRate / SubSampling);
    subSampling = SubSampling;
    samplingRate = SamplingRate;
    numCh =NumCh;
    leftover_k = 0;
    Buf.resize(numCh);


    for (int k=0; k< numCh; k++)
    {
        Buf[k].resize(numSamplesToHoldPerChannel);
    }

    hardwareTS.resize(numSamplesToHoldPerChannel);
    softwareTS.resize(numSamplesToHoldPerChannel);
    valid.resize(numSamplesToHoldPerChannel);
    bufLen = numSamplesToHoldPerChannel;
    numSamplesInBuf = 0;
    ptr = 0; // points to a valid position in the buffer.
}


void ContinuousCircularBufferWithStim::update(int channel, int64 hardware_ts, int64 software_ts, bool rise)
{
    // used to record ttl pulses as continuous data...
    mut.enter();
    valid[ptr] = true;
    hardwareTS[ptr] = hardware_ts;
    softwareTS[ptr] = software_ts;

    Buf[channel][ptr] = (rise) ? 1.0 : 0.0;

    ptr++;
    if (ptr == bufLen)
    {
        ptr = 0;
    }
    numSamplesInBuf++;
    if (numSamplesInBuf >= bufLen)
    {
        numSamplesInBuf = bufLen;
    }
    mut.exit();
}

void ContinuousCircularBufferWithStim::update(AudioSampleBuffer& buffer, int64 hardware_ts, int64 software_ts, int numpts)
{
    mut.enter();

    // we don't start from zero because of subsampling issues.
    // previous packet may not have ended exactly at the last given sample.
    int k = leftover_k;
    int lastUsedSample = 0;
    for (; k < numpts; k+=subSampling)
    {
        lastUsedSample = k;
        valid[ptr] = true;
        hardwareTS[ptr] = hardware_ts + k;
        softwareTS[ptr] = software_ts + int64(float(k) / samplingRate * numTicksPerSecond);

        for (int ch = 0; ch < numCh; ch++)
        {
            Buf[ch][ptr] = *(buffer.getReadPointer(ch,k));
        }
        ptr++;
        if (ptr == bufLen)
        {
            ptr = 0;
        }
        numSamplesInBuf++;
        if (numSamplesInBuf >= bufLen)
        {
            numSamplesInBuf = bufLen;
        }
    }

    int numMissedSamples = (numpts-1)-lastUsedSample;
    leftover_k = (subSampling-numMissedSamples-1) % subSampling;
    mut.exit();

}


void ContinuousCircularBufferWithStim::update(std::vector<std::vector<bool>> contdata, int64 hardware_ts, int64 software_ts, int numpts)
{
    mut.enter();

    // we don't start from zero because of subsampling issues.
    // previous packet may not have ended exactly at the last given sample.
    int k = leftover_k;
    int lastUsedSample = 0;
    for (; k < numpts; k+=subSampling)
    {
        lastUsedSample = k;
        valid[ptr] = true;
        hardwareTS[ptr] = hardware_ts + k;
        softwareTS[ptr] = software_ts + int64(float(k) / samplingRate * numTicksPerSecond);

        for (int ch = 0; ch < numCh; ch++)
        {
            Buf[ch][ptr] = contdata[ch][k];
        }
        ptr++;
        if (ptr == bufLen)
        {
            ptr = 0;
        }
        numSamplesInBuf++;
        if (numSamplesInBuf >= bufLen)
        {
            numSamplesInBuf = bufLen;
        }
    }

    int numMissedSamples = (numpts-1)-lastUsedSample;
    leftover_k =subSampling-numMissedSamples-1;
    mut.exit();

}
/*
void ContinuousCircularBufferWithStim::AddDataToBuffer(std::vector<std::vector<double>> lfp, double soft_ts)
{
	mut.enter();
	int numpts = lfp[0].size();
	for (int k = 0; k < numpts / subSampling; k++)
	{
		valid[ptr] = true;
		for (int ch = 0; ch < numCh; ch++)
		{
			Buf[ch][ptr] = lfp[ch][k];
			TS[ptr] = soft_ts + (double)(k * subSampling) / samplingRate;
		}
		ptr++;
		if (ptr == bufLen)
		{
			ptr = 0;
		}
		numSamplesInBuf++;
		if (numSamplesInBuf >= bufLen)
		{
			numSamplesInBuf = bufLen;
		}
	}
	mut.exit();
}
*/

int ContinuousCircularBufferWithStim::GetPtr()
{
    return ptr;
}

/************************************************************/

