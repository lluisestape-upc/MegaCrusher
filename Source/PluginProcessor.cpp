#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
MegaCrusherProcessor::MegaCrusherProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "State", createParameterLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout
MegaCrusherProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "drive",  1 }, "Drive",  0.f, 1.f, 0.f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "tone",   1 }, "Tone",   0.f, 1.f, 0.5f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "crush",  1 }, "Crush",  0.f, 1.f, 0.f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix",    1 }, "Mix",    0.f, 1.f, 1.f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 }, "Output", 0.f, 1.f, 0.75f));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "mode",   1 }, "Mode",
        juce::StringArray { "SOFT", "HARD", "FOLD" }, 0));

    return { p.begin(), p.end() };
}

//==============================================================================
bool MegaCrusherProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto mono   = juce::AudioChannelSet::mono();
    auto stereo = juce::AudioChannelSet::stereo();
    auto& out   = layouts.getMainOutputChannelSet();
    auto& in    = layouts.getMainInputChannelSet();

    if (out != stereo && out != mono) return false;
    return in == out;
}

void MegaCrusherProcessor::prepareToPlay (double sampleRate, int)
{
    sr     = sampleRate;
    float w = 2.f * juce::MathConstants<float>::pi * 2000.f / (float)sr;
    tCoeff  = w / (1.f + w);
    tsL = tsR = 0.f;
}

void MegaCrusherProcessor::releaseResources()
{
    tsL = tsR = 0.f;
}

//==============================================================================
void MegaCrusherProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    float driveGain = 1.f + apvts.getRawParameterValue ("drive") ->load() * 39.f;
    float tone      =       apvts.getRawParameterValue ("tone")  ->load();
    float crush     =       apvts.getRawParameterValue ("crush") ->load();
    float mix       =       apvts.getRawParameterValue ("mix")   ->load();
    float outGain   = juce::Decibels::decibelsToGain (
                          -24.f + apvts.getRawParameterValue ("output")->load() * 30.f);
    int   mode      = (int) apvts.getRawParameterValue ("mode")  ->load();

    float maxL = 0.f, maxR = 0.f;
    int   nCh  = juce::jmin (buffer.getNumChannels(), 2);
    int   ns   = buffer.getNumSamples();

    for (int c = 0; c < nCh; ++c)
    {
        auto*  data = buffer.getWritePointer (c);
        float& ts   = (c == 0) ? tsL : tsR;

        for (int i = 0; i < ns; ++i)
        {
            float dry = data[i];
            float wet = dry * driveGain;

            // Saturate
            switch (mode)
            {
                case 0:  wet = std::tanh (wet);                        break;
                case 1:  wet = juce::jlimit (-1.f, 1.f, wet);         break;
                case 2:
                {
                    int n = 0;
                    while ((wet > 1.f || wet < -1.f) && ++n < 64)
                    {
                        if (wet >  1.f) wet =  2.f - wet;
                        if (wet < -1.f) wet = -2.f - wet;
                    }
                    break;
                }
                default: break;
            }

            // Tone tilt  (one-pole LP blend)
            ts  += tCoeff * (wet - ts);
            float lp = ts, hp = wet - lp;
            if   (tone < 0.5f) wet = lp + (tone * 2.f) * (wet - lp);
            else               wet = wet + (tone - 0.5f) * 2.f * hp * 0.6f;

            // Bit crush
            if (crush > 0.001f)
            {
                float levels = std::pow (2.f, 2.f + (1.f - crush) * 14.f);
                wet = std::round (wet * levels) / levels;
            }

            // Dry/wet + output
            wet     = dry + mix * (wet - dry);
            wet    *= outGain;
            data[i] = wet;

            float aw = std::abs (wet);
            if (c == 0) maxL = juce::jmax (maxL, aw);
            else        maxR = juce::jmax (maxR, aw);
        }
    }

    vuL.store (maxL);
    vuR.store (maxR);
}

//==============================================================================
juce::AudioProcessorEditor* MegaCrusherProcessor::createEditor()
{
    return new MegaCrusherEditor (*this);
}

void MegaCrusherProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MegaCrusherProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MegaCrusherProcessor();
}
