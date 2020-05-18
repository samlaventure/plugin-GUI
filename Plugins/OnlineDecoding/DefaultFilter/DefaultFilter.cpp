#include <math.h>
#include <iostream>
#include "DefaultFilter.h"

#define PI 3.141592653589793

DefaultFilter::DefaultFilter() {}
DefaultFilter::DefaultFilter(float samplingRate, float cutOff) :
	sampleFreq(samplingRate), cutOffFreq(cutOff)

{
	setParameters();
}

void DefaultFilter::setSampleFreq(float samplingRate)
{
	sampleFreq = samplingRate;
	setParameters();
}

void DefaultFilter::setCutOffFreq(float cutOff)
{
	cutOffFreq = cutOff;
	setParameters();
}

void DefaultFilter::setParameters()
{
	a = exp(-2 * PI * cutOffFreq / sampleFreq);
	b = 1 - a;
	state = 0.f;
}

void DefaultFilter::processSample(float& spl)
{
	filteredSpl = spl - state;
	state = a * state + b * spl;
}

void DefaultFilter::process(uint N, float* array)
{
	for (uint i=0; i<N; ++i) {
		processSample(*(array+i));
		*(array+i) = filteredSpl;
	}
}
