#ifndef DEFAULTFILTERFILTER
#define DEFAULTFILTERFILTER

class DefaultFilter
{
public:
	DefaultFilter();
	DefaultFilter(float samplingRate, float cutOff);
	void setSampleFreq(float samplingRate);
	void setCutOffFreq(float cutOff);
	void process(uint N, float* array);

private:
	float state = 0.f;
	float a,b,filteredSpl;
	float sampleFreq, cutOffFreq;
	void setParameters();
	void processSample(float& spl);
};

#endif  // DEFAULTFILTERFILTER