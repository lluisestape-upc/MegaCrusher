#pragma once
#include <JuceHeader.h>

class MegaCrusherEditor;   // forward-declared so createEditor() can return it

class MegaCrusherProcessor : public juce::AudioProcessor
{
public:
    MegaCrusherProcessor();
    ~MegaCrusherProcessor() override = default;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout&) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==========================================================================
    const juce::String getName() const override         { return "MegaCrusher"; }
    bool  acceptsMidi()  const override                 { return false; }
    bool  producesMidi() const override                 { return false; }
    bool  isMidiEffect() const override                 { return false; }
    double getTailLengthSeconds() const override        { return 0.0; }

    //==========================================================================
    int  getNumPrograms()  override                     { return 1; }
    int  getCurrentProgram() override                   { return 0; }
    void setCurrentProgram (int) override               {}
    const juce::String getProgramName (int) override    { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    juce::AudioProcessorValueTreeState apvts;

    // VU bridge: written on audio thread, read on message thread
    std::atomic<float> vuL { 0.f }, vuR { 0.f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Audio-thread state only
    double sr     = 44100.0;
    float  tsL    = 0.f, tsR = 0.f;
    float  tCoeff = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MegaCrusherProcessor)
};
