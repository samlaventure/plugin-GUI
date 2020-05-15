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

#ifndef POSITIONDECODERPROCESSOR_H_INCLUDED
#define POSITIONDECODERPROCESSOR_H_INCLUDED

#ifdef _WIN32
#include <Windows.h>
#endif

#include <ProcessorHeaders.h>

#define WINDOW_LENGTH 0.036 // in seconds

enum SleepState : short;
class PositionDecoderNetwork;
namespace OnlineDecoding
{
    struct DecodingResults;

    struct Spike
    {
        Spike(int spkchn, int spl, int grp, int nChannels);
        int spikeChannel;
        int peakSample;
        int group;
        std::vector<std::vector<float>> samples;
        bool ready;
    };

    struct SignalBuffer: public std::vector<std::vector<float>>
    {
        float zero = 0;
        const int length = 20;
        const float getElement(uint channel, int sampleIndex) {
            return operator()(channel, sampleIndex);
        }
        float& operator()(uint channel, int sampleIndex) {
            if (sampleIndex >= 0) return this->at(channel).at(sampleIndex);
            else return this->at(channel).at(length + sampleIndex);
        }
    };    
}


class PositionDecoderProcessor : public GenericProcessor

{
public:
    /** The class constructor, used to initialize any members. */
    PositionDecoderProcessor();

    /** The class destructor, used to deallocate memory */
    ~PositionDecoderProcessor();

    /** Indicates if the processor has a custom editor. Defaults to false */
    //bool hasEditor() const { return true; }

    //TODO <Kirill A>: it was commented before, think about it
    /** If the processor has a custom editor, this method must be defined to instantiate it. */
    AudioProcessorEditor* createEditor() override;

    /** Optional method that informs the GUI if the processor is ready to function. If false acquisition cannot start. Defaults to true */
    //bool isReady();

    /** Defines the functionality of the processor.

        The process method is called every time a new data buffer is available.

        Processors can either use this method to add new data, manipulate existing
        data, or send data to an external target (such as a display or other hardware).

        Continuous signals arrive in the "buffer" variable, event data (such as TTLs
        and spikes) is contained in the "events" variable.
    */
    void decode (int ts);
    void process (AudioSampleBuffer& buffer) override;
    void handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition) override;
    void handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition) override;

    /** The method that standard controls on the editor will call.
        It is recommended that any variables used by the "process" function
        are modified only through this method while data acquisition is active. */
    void setParameter (int parameterIndex, float newValue) override;

    void createEventChannels() override;

    bool setFile(String fullPath);
    void setSleepState(SleepState newSleepState);
    void startAcquisition();

    /** Saving custom settings to XML. */
    virtual void saveCustomParametersToXml (XmlElement* parentElement) override;

    /** Load custom settings from XML*/
    virtual void loadCustomParametersFromXml() override;

    /** Optional method called every time the signal chain is refreshed or changed in any way.

        Allows the processor to handle variations in the channel configuration or any other parameter
        passed down the signal chain. The processor can also modify here the settings structure, which contains
        information regarding the input and output channels as well as other signal related parameters. Said
        structure shouldn't be manipulated outside of this method.
    */
    void updateSettings();

private:
    bool checkOutput();
    bool checkSleepState();

    String jsonPath;

    float sampleRate;
    int nSamplesInCurrentWindow;

    std::vector<int> groups;
    std::vector<std::vector<uint>> channels;
    ScopedPointer<PositionDecoderNetwork> decoder;

    int currentSleepStateLength = 0;
    SleepState sleepStateSelection, currentSleepState;

    OnlineDecoding::SignalBuffer signalBuffer;
    OwnedArray<OnlineDecoding::Spike> spikesInWindow;
    ScopedPointer<OnlineDecoding::DecodingResults> output;

    EventChannel* ttlChannel;
    int ttlMessageUp = 1;
    int ttlMessageDown = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PositionDecoderProcessor);
};

#endif  // POSITIONDECODERPROCESSOR_H_INCLUDED
