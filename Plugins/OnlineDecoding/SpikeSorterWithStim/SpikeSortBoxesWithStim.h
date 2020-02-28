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

#ifndef __SPIKESORTBOXESWITHSTIM_H
#define __SPIKESORTBOXESWITHSTIM_H

#include "SpikeSorterEditorWithStim.h"
#include <algorithm>    // std::sort
#include <list>
#include <queue>
#include <atomic>

class SorterSpikeContainerWithStim : public ReferenceCountedObject
{
public:
	//This invalidates the original SpikeEventPtr, so be careful
	SorterSpikeContainerWithStim(const SpikeChannel* channel, SpikeEvent::SpikeBuffer& data, int64 timestamp);
	SorterSpikeContainerWithStim() = delete;

	const float* getData() const;
	const SpikeChannel* getChannel() const;
	int64 getTimestamp() const;
	uint8 color[3];
	float pcProj[2];
	uint16 sortedId;
private:
	int64 timestamp;
	HeapBlock<float> data;
	const SpikeChannel* chan;
};
typedef ReferenceCountedObjectPtr<SorterSpikeContainerWithStim> SorterSpikeWithStimPtr;
typedef ReferenceCountedArray<SorterSpikeContainerWithStim, CriticalSection> SorterSpikeWithStimArray;

class PCAcomputingThreadWithStim;
class UniqueIDgeneratorWithStim;
class PointDWithStim
{
public:

    PointDWithStim();
    PointDWithStim(float x, float y);
    PointDWithStim(const PointDWithStim& P);
    const PointDWithStim operator+(const PointDWithStim& c1) const;
    PointDWithStim& operator+=(const PointDWithStim& rhs);
    PointDWithStim& operator-=(const PointDWithStim& rhs);


    const PointDWithStim operator-(const PointDWithStim& c1) const;
    const PointDWithStim operator*(const PointDWithStim& c1) const;

    float cross(PointDWithStim c) const;
    float X,Y;
};


class BoxWithStim
{
public:
    BoxWithStim();
    BoxWithStim(int channel);
    BoxWithStim(float X, float Y, float W, float H, int ch=0);
    bool LineSegmentIntersection(PointDWithStim p11, PointDWithStim p12, PointDWithStim p21, PointDWithStim p22);
    bool isWaveFormInside(SorterSpikeWithStimPtr so);
    double x,y,w,h; // x&w and specified in microseconds. y&h in microvolts
    int channel;
};


/************************/
class HistogramWithStim
{
public:
    HistogramWithStim();
    HistogramWithStim(int N, double T0, double T1);
    ~HistogramWithStim();

    void setParameters(int N, double T0, double T1);
    std::vector<int> getCounter();
    void reset();
    void update(double x);

    int Max;
    double t0, t1;
    std::vector<double> Time;
    int numBins;
    std::vector<int> Counter;

};

class RunningStatsWithStim
{
public:
    RunningStatsWithStim();
    ~RunningStatsWithStim();
    void resizeWaveform(int newlength);
    void reset();
    HistogramWithStim getHistogram();
    std::vector<double> getMean(int index);
    std::vector<double> getStandardDeviation(int index);
    void update(SorterSpikeWithStimPtr so);
    bool queryNewData();

    double LastSpikeTime;
    bool newData;
    HistogramWithStim hist;
    std::vector<std::vector<double> > WaveFormMean,WaveFormSk,WaveFormMk;
    double numSamples;


};

// BoxWithStim unit defines a single unit (with multiple boxes)
// Each boxWithStim can be on a different channel
class BoxUnitWithStim
{
public:
    BoxUnitWithStim();
    BoxUnitWithStim(int ID, int localID);
    BoxUnitWithStim(BoxWithStim B, int ID, int localID);
    bool isWaveFormInsideAllBoxes(SorterSpikeWithStimPtr so);
    bool isActivated();
    void activateUnit();
    void deactivateUnit();
    double getNumSecondsActive();
    void toggleActive();
    void addBox(BoxWithStim b);
    void addBox();
    int getNumBoxes();
    void modifyBox(int boxindex, BoxWithStim b);
    bool deleteBox(int boxindex);
    BoxWithStim getBox(int boxWithStim);
    void setBox(int boxid, BoxWithStim B);
    void setBoxPos(int boxid, PointDWithStim P);
    void setBoxSize(int boxid, double W, double H);
    void MoveBox(int boxid, int dx, int dy);
    std::vector<BoxWithStim> getBoxes();
    int getUnitID();
    int getLocalID();
	void updateWaveform(SorterSpikeWithStimPtr so);
    static void setDefaultColors(uint8_t col[3], int ID);
    void resizeWaveform(int newlength);
public:
    int UnitID;
    int localID; // used internally, for colors and position.
    std::vector<BoxWithStim> lstBoxes;
    uint8_t ColorRGB[3];
    RunningStatsWithStim WaveformStat;
    bool Active;
    juce::int64 Activated_TS_S;
    Time timer;

};

/*
class PCAjobWithStim
{
public:
PCAjobWithStim();
};*/
class PCAjobWithStim : public ReferenceCountedObject
{
public:
    PCAjobWithStim(SorterSpikeWithStimArray& _spikes, float* _pc1, float* _pc2,
           std::atomic<float>&,  std::atomic<float>&,  std::atomic<float>&,  std::atomic<float>&, std::atomic<bool>& _reportDone);
    ~PCAjobWithStim();
    void computeCov();
    void computeSVD();

    float** cov;
    SorterSpikeWithStimArray spikes;
    float* pc1, *pc2;
    std::atomic<float>& pc1min, &pc2min, &pc1max, &pc2max;
    std::atomic<bool>& reportDone;
private:
    int svdcmp(float** a, int nRows, int nCols, float* w, float** v);
    float pythag(float a, float b);
    int dim;
};

typedef ReferenceCountedObjectPtr<PCAjobWithStim> PCAJobWithStimPtr;
typedef ReferenceCountedArray<PCAjobWithStim, CriticalSection> PCAJobWithStimArray;

class cPolygonWithStim
{
public:
    cPolygonWithStim();
    bool isPointInside(PointDWithStim p);
    std::vector<PointDWithStim> pts;
    PointDWithStim offset;
};



class PCAcomputingThreadWithStim : juce::Thread
{
public:
    PCAcomputingThreadWithStim();
    void run(); // computes PCA on waveforms
    void addPCAjob(PCAJobWithStimPtr job);

private:
    PCAJobWithStimArray jobs;
	CriticalSection lock;
};

class PCAUnitWithStim
{
public:
    PCAUnitWithStim();
    PCAUnitWithStim(int ID, int localID);
    PCAUnitWithStim(cPolygonWithStim B, int ID, int localID_);
    ~PCAUnitWithStim();
    int getUnitID();
    int getLocalID();
	bool isWaveFormInsidePolygon(SorterSpikeWithStimPtr so);
    bool isPointInsidePolygon(PointDWithStim p);
    void resizeWaveform(int newlength);
	void updateWaveform(SorterSpikeWithStimPtr so);
public:
    int UnitID;
    int localID; // used internally, for colors and position.
    cPolygonWithStim poly;
    uint8_t ColorRGB[3];
    RunningStatsWithStim WaveformStat;
    bool Active;
    juce::int64 Activated_TS_S;
    Time timer;
};

// Sort spikes from a single electrode (which could have any number of channels)
// using the boxWithStim method. Any electrode could have an arbitrary number of units specified.
// Each unit is defined by a set of boxes, which can be placed on any of the given channels.
class SpikeSortBoxesWithStim
{
public:
    SpikeSortBoxesWithStim(UniqueIDgeneratorWithStim* uniqueIDgenerator_, PCAcomputingThreadWithStim* pth, int numch, double SamplingRate, int WaveFormLength);
    ~SpikeSortBoxesWithStim();

    void resizeWaveform(int numSamples);


	void projectOnPrincipalComponents(SorterSpikeWithStimPtr so);
	bool sortSpike(SorterSpikeWithStimPtr so, bool PCAfirst);
    void RePCA();
    void addPCAunit(PCAUnitWithStim unit);
    int addBoxUnit(int channel);
    int addBoxUnit(int channel, BoxWithStim B);

    void getPCArange(float& p1min,float& p2min, float& p1max,  float& p2max);
    void setPCArange(float p1min,float p2min, float p1max,  float p2max);
    void resetJobStatus();
    bool isPCAfinished();

    bool removeUnit(int unitID);

    void removeAllUnits();
    bool addBoxToUnit(int channel, int unitID);
    bool addBoxToUnit(int channel, int unitID, BoxWithStim B);
    bool removeBoxFromUnit(int unitID, int boxIndex);
    int getNumBoxes(int unitID);
    std::vector<BoxWithStim> getUnitBoxes(int unitID);
    std::vector<BoxUnitWithStim> getBoxUnits();
    std::vector<PCAUnitWithStim> getPCAUnits();

    void getUnitColor(int UnitID, uint8& R, uint8& G, uint8& B);
    void updateBoxUnits(std::vector<BoxUnitWithStim> _units);
    void updatePCAUnits(std::vector<PCAUnitWithStim> _units);
    int generateUnitID();
    int generateLocalID();
    void generateNewIDs();
    void setSelectedUnitAndBox(int unitID, int boxID);
    void getSelectedUnitAndBox(int& unitID, int& boxid);
    void saveCustomParametersToXml(XmlElement* electrodeNode);
    void loadCustomParametersFromXml(XmlElement* electrodeNode);
private:
    //void  StartCriticalSection();
    //void  EndCriticalSection();
    UniqueIDgeneratorWithStim* uniqueIDgenerator;
    int numChannels, waveformLength;
    int selectedUnit, selectedBox;
    CriticalSection mut;
    std::vector<BoxUnitWithStim> boxUnits;
    std::vector<PCAUnitWithStim> pcaUnits;
    float* pc1, *pc2;
    std::atomic<float> pc1min, pc2min, pc1max, pc2max;
    SorterSpikeWithStimArray spikeBuffer;
    int bufferSize,spikeBufferIndex;
    PCAcomputingThreadWithStim* computingThread;
    bool bPCAJobSubmitted,bPCAcomputed,bRePCA;
    std::atomic<bool> bPCAjobFinished ;


};

//Those are legacy methods from the old spike system that are must likely not needed in the new one
float spikeDataBinToMicrovolts(SorterSpikeWithStimPtr  s, int bin, int ch = 0);
float spikeDataIndexToMicrovolts(SorterSpikeWithStimPtr s, int index = 0);
float spikeTimeBinToMicrosecond(SorterSpikeWithStimPtr s, int bin, int ch = 0);
int microSecondsToSpikeTimeBin(SorterSpikeWithStimPtr s, float t, int ch = 0);


#endif // __SPIKESORTBOXESWITHSTIM_H
