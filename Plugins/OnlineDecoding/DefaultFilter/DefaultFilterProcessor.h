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

#ifndef __DEFAULTFILTER__
#define __DEFAULTFILTER__

#include <ProcessorHeaders.h>
#include <DspLib.h>

class DefaultFilter;
class DefaultFilterProcessor : public GenericProcessor
{
public:
    DefaultFilterProcessor();
    ~DefaultFilterProcessor();

    AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override { return true; }

    void process (AudioSampleBuffer& buffer) override;

    void setParameter (int parameterIndex, float newValue) override;

    void updateSettings() override;

    void saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelTypel) override;
    void loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)  override;

    double getLowCutValueForChannel  (int chan) const;
    double getHighCutValueForChannel (int chan) const;

    bool getBypassStatusForChannel (int chan) const;

    void setApplyOnADC (bool state);


private:
    void setFilterParameters (double, double, int);

    Array<double> lowCuts;
    Array<double> highCuts;

    OwnedArray<Dsp::Filter> filtersOld;
    OwnedArray<DefaultFilter> filters;
    Array<bool> shouldFilterChannel;

    bool applyOnADC;

    double defaultLowCut;
    double defaultHighCut;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DefaultFilterProcessor);
};

#endif  // __DEFAULTFILTER__
