#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Hell-themed LookAndFeel
//==============================================================================
class HellLookAndFeel : public juce::LookAndFeel_V4
{
public:
    inline static const juce::Colour Black    { 0xff080002u };
    inline static const juce::Colour Coal     { 0xff130005u };
    inline static const juce::Colour Dim      { 0xff551100u };
    inline static const juce::Colour Ember    { 0xffCC2200u };
    inline static const juce::Colour Orange   { 0xffFF5500u };
    inline static const juce::Colour Amber    { 0xffFF9900u };
    inline static const juce::Colour WhiteHot { 0xffFFDD88u };

    HellLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float pos, float startA, float endA, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour&, bool hi, bool down) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&, bool hi, bool down) override;
};

//==============================================================================
// Ember particle
//==============================================================================
struct Ember { float x, y, size, speed, opacity, wobble, phase; };

//==============================================================================
// VU meter
//==============================================================================
class VUMeter : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
    void setLevel (float leftDb, float rightDb);
    void decayPeak();

private:
    float lL = -60.f, lR = -60.f, pL = -60.f, pR = -60.f;
    int   hL = 0, hR = 0;
    static constexpr int kHold = 45;

    void drawChan (juce::Graphics&, juce::Rectangle<int>, float lv, float pk);
};

//==============================================================================
// Transfer-function curve
//==============================================================================
class TransferCurve : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
    void setParams (float drive, int mode);

private:
    float drive = 0.f;
    int   mode  = 0;
    float eval  (float x) const;
};

//==============================================================================
// Plugin editor
//==============================================================================
class MegaCrusherEditor : public juce::AudioProcessorEditor,
                          private juce::Timer
{
public:
    explicit MegaCrusherEditor (MegaCrusherProcessor&);
    ~MegaCrusherEditor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    void timerCallback() override;
    void syncModeButtons();

    MegaCrusherProcessor& proc;

    HellLookAndFeel lnf;

    juce::Slider     driveKnob, toneKnob, crushKnob, mixKnob, outKnob;
    juce::Label      driveLabel, toneLabel, crushLabel, mixLabel, outLabel;
    juce::TextButton softBtn { "SOFT" }, hardBtn { "HARD" }, foldBtn { "FOLD" };

    VUMeter       vu;
    TransferCurve curve;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> driveAtt, toneAtt, crushAtt, mixAtt, outAtt;

    juce::Random       rng;
    std::vector<Ember> embers;
    float              animT  = 0.f;
    float              vuSL   = -60.f, vuSR = -60.f;

    void initEmbers();
    void tickEmbers();
    void drawBg     (juce::Graphics&);
    void drawFire   (juce::Graphics& g, float drive);
    void drawEmbers (juce::Graphics& g, float drive);
    void drawTitle  (juce::Graphics&);
    void setupKnob  (juce::Slider&, juce::Label&, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MegaCrusherEditor)
};
