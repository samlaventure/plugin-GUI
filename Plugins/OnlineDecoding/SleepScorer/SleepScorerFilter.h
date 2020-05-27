#ifndef SLEEPSCORERFILTER_H
#define SLEEPSCORERFILTER_H

#include <vector>
#include <fftw3.h>
#include <ProcessorHeaders.h>

#define REAL 0
#define IMAG 1
#define DOWN_SAMPLING_FACTOR 60
#define SLEEP_SCORING_WINDOW_SIZE 3.
#define SLEEP_SCORING_FREQ 3 // as a fraction of SLEEP_SCORING_WINDOW_SIZE

class Fir1;

enum FilterType {
	Gamma,
	Theta,
	Delta,
	UndeclaredRhythm
};

enum SleepState : short {
	undefinedSleepState = 1, // We avoid zeros to use these IDs in a juce::ComboBox
	Wake,
	NREM,
	REM,
	Sleep
};

struct SleepScorerFilter
{
	SleepScorerFilter();
	SleepScorerFilter(float inSampleRate, FilterType inFilterType);
	SleepScorerFilter(const SleepScorerFilter &that);
	SleepScorerFilter& operator=(SleepScorerFilter that);
	~SleepScorerFilter();
	friend void swap(SleepScorerFilter& first, SleepScorerFilter& second);

	bool filterREADY = false, jobReady = false, shouldSwap = false, newSleepScoringIsReady = false;
	void setFilter();

	bool addSampleAndCheckWindow(float newSample);
	const bool isNewSleepScoringReady() {return newSleepScoringIsReady;}

	void computeRhythmPower();
	void computeRhythmPower(SleepScorerFilter *otherFilter);
	void computeHilbertTransform();
	void computeFilter();
	float rhythmPower();
	void incrementIdx();
	void clear();

	float sampleRate;
	long length, halfLength, remaining;

	FilterType filterType;
	std::vector<float> signal, signalBuffer;
	std::vector<float> rhythmAmplitude;
	int powerArrayIdx = 0, signalIndex = 0;
	float powerArray[SLEEP_SCORING_FREQ];

	ScopedPointer<Fir1> Filter;
	fftw_complex *rhythmHilbertIn, *rhythmHilbertOut;
	fftw_plan    rhythmForwardFft, rhythmBackwardFft;

};

class SleepScoringThread : juce::Thread
{
public:
	SleepScoringThread(const bool& inFiltersReady);
	void setFilters(SleepScorerFilter* inGammafilter, SleepScorerFilter* inThetaFilter, SleepScorerFilter* inDeltaFilter);
	void run() override;
private:
	const bool& filtersReady;
	SleepScorerFilter *gammaFilter=0, *thetaFilter=0, *deltaFilter=0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SleepScoringThread)
};


#endif // SLEEPSCORERFILTER_H