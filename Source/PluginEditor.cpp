#include "PluginEditor.h"
#include <cmath>

//==============================================================================
// HellLookAndFeel

HellLookAndFeel::HellLookAndFeel()
{
    setColour (juce::TextButton::buttonColourId,   Coal);
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff220800u));
    setColour (juce::TextButton::textColourOffId,  Orange);
    setColour (juce::TextButton::textColourOnId,   WhiteHot);
    setColour (juce::Label::textColourId,          Orange);
    setColour (juce::Label::backgroundColourId,    juce::Colours::transparentBlack);
}

void HellLookAndFeel::drawRotarySlider (juce::Graphics& g,
    int x, int y, int w, int h,
    float pos, float startA, float endA, juce::Slider&)
{
    // Scale glow based on knob size to avoid clipping artefacts
    bool  large     = (w >= 110);
    float reduction = large ? 6.f : 11.f;
    float glowBase  = large ? 4.f  : 2.f;
    float glowDrive = large ? 10.f : 5.f;
    float glowStep  = large ? 3.f  : 2.f;

    auto  bounds = juce::Rectangle<float> (x, y, w, h).reduced (reduction);
    float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float cx = bounds.getCentreX(), cy = bounds.getCentreY();
    float angle = startA + pos * (endA - startA);

    // Outer glow halo  (clamped to stay within component bounds)
    float maxExtra = (float)w * 0.5f - radius - 1.f;
    float glowExtra = juce::jmin (maxExtra, glowBase + pos * glowDrive);
    for (int i = 3; i >= 1; --i)
    {
        float r = radius + glowExtra + (float)i * juce::jmin (maxExtra * 0.35f, glowStep);
        g.setColour (Orange.withAlpha ((0.05f + 0.08f * pos) / (float)i));
        g.fillEllipse (cx - r, cy - r, r * 2.f, r * 2.f);
    }

    // Metallic outer ring
    {
        juce::ColourGradient ring (juce::Colour (0xff281010u), cx, cy - radius,
                                   juce::Colour (0xff090303u), cx, cy + radius, false);
        g.setGradientFill (ring);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.f, radius * 2.f);
    }

    // Dark inner body
    float ir = radius * 0.80f;
    {
        juce::ColourGradient body (juce::Colour (0xff1c0808u), cx, cy - ir,
                                   juce::Colour (0xff040000u), cx, cy + ir, false);
        g.setGradientFill (body);
        g.fillEllipse (cx - ir, cy - ir, ir * 2.f, ir * 2.f);
    }

    float trackR = radius * 0.87f;

    // Track arc
    {
        juce::Path t;
        t.addArc (cx - trackR, cy - trackR, trackR * 2.f, trackR * 2.f,
                  startA, endA, true);
        g.setColour (Dim.withAlpha (0.5f));
        g.strokePath (t, juce::PathStrokeType (2.f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    // Value arc with glow
    if (pos > 0.002f)
    {
        juce::Path v;
        v.addArc (cx - trackR, cy - trackR, trackR * 2.f, trackR * 2.f,
                  startA, angle, true);

        g.setColour (Orange.withAlpha (0.22f));
        g.strokePath (v, juce::PathStrokeType (6.f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
        g.setColour (Ember.interpolatedWith (Amber, pos));
        g.strokePath (v, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    // Pointer
    float pl = ir * 0.68f;
    juce::Point<float> tip  { cx + pl         * std::sin (angle), cy - pl         * std::cos (angle) };
    juce::Point<float> base { cx + ir * 0.12f * std::sin (angle), cy - ir * 0.12f * std::cos (angle) };

    g.setColour (Orange.withAlpha (0.45f));
    g.drawLine (base.x, base.y, tip.x, tip.y, 3.5f);
    g.setColour (WhiteHot);
    g.drawLine (base.x, base.y, tip.x, tip.y, 1.8f);

    float dotR = ir * 0.07f;
    g.setColour (juce::Colour (0xff3a0808u));
    g.fillEllipse (cx - dotR, cy - dotR, dotR * 2.f, dotR * 2.f);
}

void HellLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& btn,
    const juce::Colour&, bool hi, bool down)
{
    auto  b  = btn.getLocalBounds().toFloat().reduced (1.f);
    bool  on = btn.getToggleState();

    if (on)
    {
        g.setColour (Orange.withAlpha (0.20f));
        g.fillRoundedRectangle (b.expanded (3.f), 6.f);
    }

    auto bg = on ? juce::Colour (0xff220800u) : Black;
    if (hi || down) bg = bg.brighter (0.12f);
    g.setColour (bg);
    g.fillRoundedRectangle (b, 4.f);

    g.setColour (on ? Orange : Dim);
    g.drawRoundedRectangle (b, 4.f, on ? 1.5f : 1.f);
}

void HellLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& btn, bool hi, bool)
{
    bool on = btn.getToggleState();
    g.setColour (on ? WhiteHot : (hi ? Amber : Orange));
    g.setFont (juce::FontOptions{}.withHeight (11.f).withStyle ("Bold"));
    g.drawText (btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred);
}

//==============================================================================
// VUMeter

void VUMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();
    g.fillAll (juce::Colour (0xff050001u));
    g.setColour (juce::Colour (0xff330008u));
    g.drawRect (b, 1);

    int cw = (b.getWidth() - 5) / 2;
    drawChan (g, b.removeFromLeft (cw), lL, pL);
    b.removeFromLeft (5);
    drawChan (g, b, lR, pR);
}

void VUMeter::drawChan (juce::Graphics& g, juce::Rectangle<int> a, float lv, float pk)
{
    const float minDb = -60.f, maxDb = 0.f;
    const int   N     = 22;
    int sh = juce::jmax (1, (a.getHeight() - 2) / N);

    for (int i = 0; i < N; ++i)
    {
        float segDb = maxDb - (float)i / (float)(N - 1) * (maxDb - minDb);
        bool  act   = (lv >= segDb);
        float t     = 1.f - (float)i / (float)N;

        juce::Colour c;
        if      (t > 0.85f) c = act ? juce::Colour (0xffFFDD88u) : juce::Colour (0xff1a0606u);
        else if (t > 0.65f) c = act ? juce::Colour (0xffFF8800u) : juce::Colour (0xff150404u);
        else if (t > 0.40f) c = act ? juce::Colour (0xffFF3300u) : juce::Colour (0xff110303u);
        else                c = act ? juce::Colour (0xffAA1100u) : juce::Colour (0xff0e0202u);

        g.setColour (c);
        g.fillRect (a.getX() + 1, a.getY() + 1 + i * sh, a.getWidth() - 2, sh - 1);
    }

    if (pk > minDb)
    {
        float t  = (pk - minDb) / (maxDb - minDb);
        int   py = a.getY() + 1 + (int)((1.f - t) * (float)(a.getHeight() - 2));
        g.setColour (juce::Colour (0xffFFDD88u));
        g.fillRect (a.getX() + 1, py, a.getWidth() - 2, 2);
    }
}

void VUMeter::setLevel (float ld, float rd)
{
    lL = ld; lR = rd;
    if (ld > pL) { pL = ld; hL = kHold; }
    if (rd > pR) { pR = rd; hR = kHold; }
}

void VUMeter::decayPeak()
{
    if (hL > 0) --hL; else pL -= 1.2f;
    if (hR > 0) --hR; else pR -= 1.2f;
    pL = juce::jmax (pL, -60.f);
    pR = juce::jmax (pR, -60.f);
    repaint();
}

//==============================================================================
// TransferCurve

void TransferCurve::setParams (float d, int m)
{
    drive = d; mode = m;
    repaint();
}

float TransferCurve::eval (float x) const
{
    float gain = 1.f + drive * 39.f;
    float d    = x * gain;

    switch (mode)
    {
        case 0:
        {
            float norm = std::tanh (gain * 0.5f);
            return norm > 0.f ? std::tanh (d) / norm : std::tanh (d);
        }
        case 1:
            return juce::jlimit (-1.f, 1.f, d);
        case 2:
        {
            float v = d;
            int   n = 0;
            while ((v > 1.f || v < -1.f) && ++n < 64)
            {
                if (v >  1.f) v =  2.f - v;
                if (v < -1.f) v = -2.f - v;
            }
            return v;
        }
        default: return x;
    }
}

void TransferCurve::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();
    g.fillAll (juce::Colour (0xff050001u));

    g.setColour (juce::Colour (0xff1a0505u));
    g.drawLine ((float)b.getX(),      (float)b.getCentreY(), (float)b.getRight(),  (float)b.getCentreY(), 1.f);
    g.drawLine ((float)b.getCentreX(),(float)b.getY(),       (float)b.getCentreX(),(float)b.getBottom(),  1.f);

    g.setColour (juce::Colour (0xff1f0606u));
    g.drawLine ((float)b.getX(), (float)b.getBottom(), (float)b.getRight(), (float)b.getY(), 1.f);

    juce::Path p;
    int   W  = b.getWidth();
    float cy = (float)b.getCentreY();
    float hw = (float)b.getHeight() * 0.46f;

    for (int i = 0; i < W; ++i)
    {
        float nx  = (float)i / (float)(W - 1);
        float out = eval (nx * 2.f - 1.f);
        float sx  = (float)b.getX() + nx * (float)W;
        float sy  = cy - out * hw;
        if (i == 0) p.startNewSubPath (sx, sy);
        else        p.lineTo (sx, sy);
    }

    g.setColour (juce::Colour (0xffFF5500u).withAlpha (0.25f));
    g.strokePath (p, juce::PathStrokeType (4.f));
    g.setColour (juce::Colour (0xffFF9900u));
    g.strokePath (p, juce::PathStrokeType (1.8f));

    g.setColour (juce::Colour (0xff330008u));
    g.drawRect (b, 1);
}

//==============================================================================
// MegaCrusherEditor

MegaCrusherEditor::MegaCrusherEditor (MegaCrusherProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setSize (500, 500);
    setLookAndFeel (&lnf);

    setupKnob (driveKnob, driveLabel, "DRIVE");
    setupKnob (toneKnob,  toneLabel,  "TONE");
    setupKnob (crushKnob, crushLabel, "CRUSH");
    setupKnob (mixKnob,   mixLabel,   "MIX");
    setupKnob (outKnob,   outLabel,   "OUTPUT");

    driveAtt = std::make_unique<SliderAttachment> (proc.apvts, "drive",  driveKnob);
    toneAtt  = std::make_unique<SliderAttachment> (proc.apvts, "tone",   toneKnob);
    crushAtt = std::make_unique<SliderAttachment> (proc.apvts, "crush",  crushKnob);
    mixAtt   = std::make_unique<SliderAttachment> (proc.apvts, "mix",    mixKnob);
    outAtt   = std::make_unique<SliderAttachment> (proc.apvts, "output", outKnob);

    // Refresh transfer curve when drive changes
    driveKnob.onValueChange = [this]
    {
        curve.setParams ((float)driveKnob.getValue(),
                         (int)proc.apvts.getRawParameterValue ("mode")->load());
    };

    // Mode buttons
    auto setupMode = [this] (juce::TextButton& btn, int m)
    {
        btn.setClickingTogglesState (true);
        btn.setRadioGroupId (1);
        btn.onClick = [this, &btn, m]
        {
            auto* p = dynamic_cast<juce::AudioParameterChoice*> (
                          proc.apvts.getParameter ("mode"));
            if (p) { p->beginChangeGesture(); *p = m; p->endChangeGesture(); }
            curve.setParams ((float)driveKnob.getValue(), m);
        };
        addAndMakeVisible (btn);
    };
    setupMode (softBtn, 0);
    setupMode (hardBtn, 1);
    setupMode (foldBtn, 2);

    addAndMakeVisible (vu);
    addAndMakeVisible (curve);

    syncModeButtons();
    curve.setParams (0.f, 0);
    initEmbers();
    startTimerHz (30);
}

MegaCrusherEditor::~MegaCrusherEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void MegaCrusherEditor::setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (juce::FontOptions{}.withHeight (10.f).withStyle ("Bold")));
    l.setColour (juce::Label::textColourId,       juce::Colour (0xff994422u));
    l.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (l);
}

void MegaCrusherEditor::syncModeButtons()
{
    int m = (int)proc.apvts.getRawParameterValue ("mode")->load();
    softBtn.setToggleState (m == 0, juce::dontSendNotification);
    hardBtn.setToggleState (m == 1, juce::dontSendNotification);
    foldBtn.setToggleState (m == 2, juce::dontSendNotification);
}

//==============================================================================
// Animation

void MegaCrusherEditor::initEmbers()
{
    embers.clear();
    embers.resize (50);
    float fw = 500.f, fh = 500.f;

    for (auto& e : embers)
    {
        e.x       = rng.nextFloat() * fw;
        e.y       = rng.nextFloat() * fh;
        e.size    = 0.7f + rng.nextFloat() * 2.2f;
        e.speed   = 0.3f + rng.nextFloat() * 0.9f;
        e.opacity = 0.1f + rng.nextFloat() * 0.5f;
        e.wobble  = rng.nextFloat() * 0.6f;
        e.phase   = rng.nextFloat() * juce::MathConstants<float>::twoPi;
    }
}

void MegaCrusherEditor::tickEmbers()
{
    animT += 1.f / 30.f;
    float fw = (float)getWidth(), fh = (float)getHeight();
    float drive     = proc.apvts.getRawParameterValue ("drive")->load();
    float speedMult = 1.f + drive * 2.5f;

    for (auto& e : embers)
    {
        e.y     -= e.speed * speedMult;
        e.phase += (0.05f + e.wobble * 0.03f) * speedMult;
        e.x     += std::sin (e.phase) * e.wobble;

        if (e.y < -5.f)
        {
            e.x     = rng.nextFloat() * fw;
            e.y     = fh + 3.f;
            e.size  = 0.7f + rng.nextFloat() * 2.2f;
            e.speed = 0.3f + rng.nextFloat() * 0.9f;
        }
        e.x = juce::jlimit (-5.f, fw + 5.f, e.x);
    }
}

void MegaCrusherEditor::drawBg (juce::Graphics& g)
{
    auto  b  = getLocalBounds().toFloat();
    float cx = b.getCentreX();

    juce::ColourGradient bg (juce::Colour (0xff0c0002u), cx, b.getHeight(),
                              juce::Colour (0xff050001u), cx, 0.f, false);
    g.setGradientFill (bg);
    g.fillAll();

    float pulse = 0.04f + 0.015f * std::sin (animT * 0.9f);
    float gR    = b.getWidth() * 0.75f;
    juce::ColourGradient glow (juce::Colour (0xffBB1100u).withAlpha (pulse),
                                cx, b.getHeight(),
                                juce::Colours::transparentBlack,
                                cx, b.getHeight() - gR, true);
    g.setGradientFill (glow);
    g.fillRect (b);
}

void MegaCrusherEditor::drawFire (juce::Graphics& g, float drive)
{
    if (drive < 0.02f) return;

    float fw = (float)getWidth(), fh = (float)getHeight();

    // Three flame layers:  { frequency, speed, height fraction, alpha }
    struct Layer { float freq, speed, hFrac, alpha; };
    Layer layers[] = {
        { 3.5f,  2.2f, 0.55f, 0.09f },   // wide slow base
        { 6.5f,  4.0f, 0.42f, 0.13f },   // mid layer
        { 10.f,  6.5f, 0.28f, 0.17f },   // fast narrow tips
    };

    for (auto& ly : layers)
    {
        float flameH = fh * ly.hFrac * drive;
        const int N  = 60;

        juce::Path p;
        p.startNewSubPath (0.f, fh);

        for (int i = 0; i <= N; ++i)
        {
            float t  = (float)i / (float)N;
            float fx = t * fw;

            // Layered sine wobble for organic flame edge
            float wobble = std::sin (t * ly.freq       + animT * ly.speed)        * 18.f * drive
                         + std::sin (t * ly.freq * 1.7f + animT * ly.speed * 1.4f) * 10.f * drive;

            // Slowly-animated height envelope (slightly taller in centre)
            float env = 0.65f + 0.35f * std::sin (t * juce::MathConstants<float>::pi
                                                    + animT * 0.4f);
            float fy  = fh - flameH * env + wobble;
            p.lineTo (fx, fy);
        }

        p.lineTo (fw, fh);
        p.closeSubPath();

        juce::ColourGradient grad (
            juce::Colour (0xffFF6600u).withAlpha (ly.alpha * drive),
            fw * 0.5f, fh - flameH,
            juce::Colour (0xff990000u).withAlpha (ly.alpha * 0.25f * drive),
            fw * 0.5f, fh, false);
        g.setGradientFill (grad);
        g.fillPath (p);
    }
}

void MegaCrusherEditor::drawEmbers (juce::Graphics& g, float drive)
{
    float fh         = (float)getHeight();
    float driveScale = 0.2f + 0.8f * drive;

    for (const auto& e : embers)
    {
        float yFade     = juce::jlimit (0.f, 1.f, e.y / fh * 1.4f);
        float alpha     = e.opacity * yFade * driveScale;
        if (alpha < 0.01f) continue;

        float heat      = e.speed / 1.2f;
        float sizeScale = 1.f + drive;
        float r         = e.size * sizeScale;

        g.setColour (juce::Colour (0xffAA1100u)
                       .interpolatedWith (juce::Colour (0xffFF8800u), heat)
                       .withAlpha (alpha));
        g.fillEllipse (e.x - r, e.y - r, r * 2.f, r * 2.f);
    }
}

void MegaCrusherEditor::drawTitle (juce::Graphics& g)
{
    auto  area  = getLocalBounds().removeFromTop (70).toFloat();
    float pulse = 0.5f + 0.5f * std::sin (animT * 1.8f);

    g.setColour (juce::Colour (0xffBB1100u).withAlpha (0.11f + 0.05f * pulse));
    g.fillRect (area);

    g.setFont (juce::FontOptions{}.withName ("Impact").withHeight (36.f));

    float ga = 0.13f + 0.07f * pulse;
    g.setColour (juce::Colour (0xffFF4400u).withAlpha (ga));
    int offsets[] = { -2, 2 };
    for (int d : offsets)
    {
        g.drawText ("MEGACRUSHER", area.translated ((float)d, 0.f), juce::Justification::centred);
        g.drawText ("MEGACRUSHER", area.translated (0.f, (float)d), juce::Justification::centred);
    }

    g.setColour (juce::Colour (0xffFF6600u).interpolatedWith (
                     juce::Colour (0xffFFAA00u), 0.3f + 0.3f * pulse));
    g.drawText ("MEGACRUSHER", area, juce::Justification::centred);

    // Separator  — no subtitle text
    g.setColour (juce::Colour (0xff330008u));
    g.drawLine (20.f, 68.f, area.getRight() - 20.f, 68.f, 1.f);
}

//==============================================================================
// Component

void MegaCrusherEditor::paint (juce::Graphics& g)
{
    float drive = proc.apvts.getRawParameterValue ("drive")->load();

    drawBg     (g);
    drawFire   (g, drive);
    drawEmbers (g, drive);
    drawTitle  (g);
}

void MegaCrusherEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (70);   // title

    // Mode buttons
    {
        auto row = area.removeFromTop (36).reduced (110, 5);
        int  bw  = row.getWidth() / 3;
        softBtn.setBounds (row.removeFromLeft (bw).reduced (3, 0));
        hardBtn.setBounds (row.removeFromLeft (bw).reduced (3, 0));
        foldBtn.setBounds (row.reduced (3, 0));
    }

    area.removeFromTop (6);

    // Drive knob  —  large and centred
    {
        auto row = area.removeFromTop (162);
        int  ks  = 140, cx = row.getCentreX();
        driveKnob.setBounds (cx - ks / 2, row.getY(), ks, ks);
        driveLabel.setBounds (cx - 40, row.getY() + ks + 3, 80, 14);
        driveLabel.setFont (juce::Font (juce::FontOptions{}.withHeight (11.f).withStyle ("Bold")));
        driveLabel.setColour (juce::Label::textColourId, juce::Colour (0xffBB5522u));
    }

    area.removeFromTop (6);

    // Four secondary knobs  —  smaller, evenly spaced using float math
    {
        auto  row = area.removeFromTop (88);
        int   ks  = 64;
        float fw  = (float)row.getWidth();
        float ry  = (float)row.getY();

        for (int i = 0; i < 4; ++i)
        {
            float cellCx = (float)row.getX() + (float)i * fw / 4.f + fw / 8.f;
            int   cx     = (int)cellCx;

            juce::Slider* knobs[]  = { &toneKnob, &crushKnob, &mixKnob, &outKnob };
            juce::Label*  labels[] = { &toneLabel, &crushLabel, &mixLabel, &outLabel };

            knobs [i]->setBounds (cx - ks / 2, (int)ry,          ks, ks);
            labels[i]->setBounds (cx - 28,     (int)ry + ks + 2, 56, 12);
        }
    }

    area.removeFromTop (12);

    // Transfer curve (left) + VU meter (right)
    {
        auto row = area.removeFromTop (110);
        curve.setBounds (row.removeFromLeft (310).reduced (14, 0));
        row.removeFromLeft (4);
        vu.setBounds (row.reduced (6, 0));
    }
}

//==============================================================================
// Timer

void MegaCrusherEditor::timerCallback()
{
    tickEmbers();
    syncModeButtons();

    float rawL = proc.vuL.load(), rawR = proc.vuR.load();
    float dbL  = rawL > 0.f ? juce::Decibels::gainToDecibels (rawL) : -60.f;
    float dbR  = rawR > 0.f ? juce::Decibels::gainToDecibels (rawR) : -60.f;

    float sm = 0.65f;
    vuSL = vuSL * sm + dbL * (1.f - sm);
    vuSR = vuSR * sm + dbR * (1.f - sm);

    vu.setLevel (vuSL, vuSR);
    vu.decayPeak();

    repaint();
}
