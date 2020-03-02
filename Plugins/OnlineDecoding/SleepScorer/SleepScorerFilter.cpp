#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Fir1.h>
#include <fftw3.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <functional>
#include <algorithm>
#include <numeric>

#include "SleepScorerFilter.h"



void swap(SleepScorerFilter& first, SleepScorerFilter& second)
{
	using std::swap;
	swap(first.length, second.length);
	swap(first.halfLength, second.halfLength);
	swap(first.sampleRate, second.sampleRate);

	swap(first.filterType, second.filterType);
	swap(first.rhythmHilbertIn, second.rhythmHilbertIn);
	swap(first.rhythmHilbertOut, second.rhythmHilbertOut);
	swap(first.rhythmForwardFft, second.rhythmForwardFft);
	swap(first.rhythmBackwardFft, second.rhythmBackwardFft);

	swap(first.filterREADY, second.filterREADY);

}




SleepScorerFilter::SleepScorerFilter() 
		: sampleRate(0)
{
	rhythmHilbertIn = 0;
	rhythmHilbertOut = 0;
	filterType = UndeclaredRhythm;
}
SleepScorerFilter::SleepScorerFilter(float inSampleRate, FilterType inFilterType) 
		: sampleRate(inSampleRate), filterType(inFilterType)
{
	rhythmHilbertIn = 0;
	rhythmHilbertOut = 0;
}
// Copy constructor
SleepScorerFilter::SleepScorerFilter(const SleepScorerFilter &that) 
		: sampleRate(that.sampleRate), filterType(that.filterType)
{
	rhythmHilbertIn = 0;
	rhythmHilbertOut = 0;

	if (that.filterREADY) {
		setFilter();
	} 
}
// Copy assignment operator
SleepScorerFilter& SleepScorerFilter::operator=(SleepScorerFilter that)
{
	swap(*this, that);
	return *this;
}
// Destructor
SleepScorerFilter::~SleepScorerFilter()
{
	clear();

	fftw_free(rhythmHilbertIn);
	fftw_free(rhythmHilbertOut);
	if (filterREADY) {
		fftw_destroy_plan(rhythmForwardFft);
		fftw_destroy_plan(rhythmBackwardFft);
	}

}

void SleepScorerFilter::setFilter()
{
	float newSamplingRate = sampleRate / DOWN_SAMPLING_FACTOR;
	float slidingWindowStep = SLEEP_SCORING_WINDOW_SIZE / SLEEP_SCORING_FREQ;
	length = ((int) (newSamplingRate * slidingWindowStep) + 1);
	halfLength = length >> 1;

	signal          = std::vector<float>(length, 0.);
	signalBuffer    = std::vector<float>(length, 0.);
	rhythmAmplitude = std::vector<float>(length, 0.);
	

	for (int win=0 ; win<SLEEP_SCORING_FREQ ; ++win) {
		powerArray[win] = 0;
	}
    

	// The optimisation of fft performance will sometimes take a few seconds
    rhythmHilbertIn  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * length);
    rhythmHilbertOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * length);
    rhythmForwardFft  = fftw_plan_dft_1d(length, rhythmHilbertIn, rhythmHilbertOut, FFTW_FORWARD, FFTW_ESTIMATE);
    rhythmBackwardFft = fftw_plan_dft_1d(length, rhythmHilbertOut, rhythmHilbertOut, FFTW_BACKWARD, FFTW_ESTIMATE);



    if (filterType == UndeclaredRhythm) return;

    char  path[100] = "";
    strcat(path,"../Plugins/OnlineDecoding/SleepScorer/sleepScoringFilters/");

    switch(filterType) {
    case UndeclaredRhythm:
		return;
    case Gamma: 
    	strcat(path,"gammaFir332.fir");
    	break;
    case Theta: 
    	strcat(path,"thetaFir332.fir");
    	break;
    case Delta: 
    	strcat(path,"deltaFir332.fir");
    	break;
    default:
		return;
    }
    setlocale(LC_NUMERIC, "C");
    Filter = new Fir1(path);
    Filter->reset();
    // setlocale(LC_NUMERIC, "");



	filterREADY = true;
	std::cout << "setting up sleep scoring filter of type " << filterType << std::endl;
}


bool SleepScorerFilter::addSampleAndCheckWindow(float newSample)
{
	// shouldSwap is set to true when a sleep scoring is over.
	if (shouldSwap) {
		jobReady = false;
		shouldSwap = false;
		newSleepScoringIsReady = true;
		swap(signal, signalBuffer);
	}

	if (!jobReady) {
		// Means that the array "signal" is not full
		signal[signalIndex++] = newSample;

		if (signalIndex == length) {
			signalIndex = 0;
			jobReady = true;
			return true;
		}
		return false;

	} else {
		// We fill a buffer until sleep scoring is over and we can go back to filling the "signal" array
		if (signalIndex >= length) {
			std::cerr << "delay superior to 1 sec. Lost some sleep scoring data."<<std::endl; // should never happen
		} else {
			signalBuffer[signalIndex++] = newSample;
		}
	}
	return false;
}


void SleepScorerFilter::computeRhythmPower() 
{
	if (!filterREADY) return;
	
	computeFilter();
	computeHilbertTransform();

	powerArray[powerArrayIdx] = std::accumulate(rhythmAmplitude.begin(), rhythmAmplitude.end(), 0.0f) / length;
	incrementIdx();
	shouldSwap = true;
}

void SleepScorerFilter::computeRhythmPower(SleepScorerFilter* otherFilter)
{
	if (!filterREADY) return;
	if (filterType != Delta or otherFilter->filterType != Theta) {
		computeRhythmPower();
		return;
	}

	computeFilter();
	computeHilbertTransform();

	std::transform(
		otherFilter->rhythmAmplitude.begin(), otherFilter->rhythmAmplitude.end(),
		rhythmAmplitude.begin(), rhythmAmplitude.begin(), std::divides<float>());

	powerArray[powerArrayIdx] = std::accumulate(rhythmAmplitude.begin(), rhythmAmplitude.end(), 0.0f) / length;
	incrementIdx();
	shouldSwap = true;
}

void SleepScorerFilter::computeFilter()
{
	long pos = 0;
	while (pos < length) {

		rhythmHilbertIn[pos][REAL] = Filter->filter(signal[pos]);
		rhythmHilbertIn[pos][IMAG] = 0;
		++pos;
	}
}

void SleepScorerFilter::computeHilbertTransform()
{
	// We first apply a fft to get in frequency space
	fftw_execute(rhythmForwardFft);

	// We multiply all positive frequencies by 2
	remaining = halfLength;
	for (int i=1 ; i<halfLength ; ++i) {
		rhythmHilbertOut[i][REAL] *= 2;
		rhythmHilbertOut[i][IMAG] *= 2;
	}
	if (length%2 == 0) remaining--;
	else if (length > 1) {
		rhythmHilbertOut[halfLength][REAL] *= 2;
		rhythmHilbertOut[halfLength][IMAG] *= 2;
	}

	// We set all negative frequencies to 0
	memset(&rhythmHilbertOut[halfLength + 1][REAL], 0, remaining*sizeof(fftw_complex));

	// We apply a backward fft to get the analytic signal
	fftw_execute(rhythmBackwardFft);

	for (int i=0 ; i<length ; ++i) {
		rhythmAmplitude[i] = sqrt(rhythmHilbertOut[i][REAL] * rhythmHilbertOut[i][REAL] + 
								 rhythmHilbertOut[i][IMAG] * rhythmHilbertOut[i][IMAG]) / length;
		if (filterType==Delta && rhythmAmplitude[i]<0.01) rhythmAmplitude[i] = 0.01;
	}
}


float SleepScorerFilter::rhythmPower()
{
	float sum = 0.;
	for (int win=0 ; win<SLEEP_SCORING_FREQ ; ++win) {
		sum += powerArray[win];
	}
	newSleepScoringIsReady = false;
	return sum / SLEEP_SCORING_FREQ;
}

void SleepScorerFilter::incrementIdx()
{
	powerArrayIdx = (powerArrayIdx+1) % SLEEP_SCORING_FREQ;
}

void SleepScorerFilter::clear()
{
	for (int win=0 ; win<SLEEP_SCORING_FREQ ; ++win) {
		powerArray[win] = 0;
	}
	signal = std::vector<float>(signal.size(), 0.);
	rhythmAmplitude = std::vector<float>(rhythmAmplitude.size(), 0.);
	if (Filter!=nullptr) {
		Filter->reset();
	}
}








SleepScoringThread::SleepScoringThread(const bool& inFiltersReady)
		: Thread("sleep scoring thread"), filtersReady(inFiltersReady)
{
}

void SleepScoringThread::setFilters(SleepScorerFilter* inGammafilter, SleepScorerFilter* inThetafilter, SleepScorerFilter* inDeltaFilter)
{
	gammaFilter = inGammafilter;
	thetaFilter = inThetafilter;
	deltaFilter = inDeltaFilter;
	
	if (!isThreadRunning()) {
    	startThread();
    }
}


void SleepScoringThread::run()
{
	if (gammaFilter==nullptr or thetaFilter==nullptr or deltaFilter==nullptr) return;
	while (!threadShouldExit()) {
		wait(50);

		const MessageManagerLock mml (Thread::getCurrentThread());
		if (!mml.lockWasGained()) return;
		if (gammaFilter->jobReady) gammaFilter->computeRhythmPower();
		if (thetaFilter->jobReady) thetaFilter->computeRhythmPower();
		if (deltaFilter->jobReady) deltaFilter->computeRhythmPower(thetaFilter);
	}
}
