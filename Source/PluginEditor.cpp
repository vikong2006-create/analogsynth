#include "PluginEditor.h"

namespace
{
    using Fmt = std::function<juce::String (double)>;

    Fmt num (int decimals, juce::String suffix = {})
    {
        return [decimals, suffix] (double v) { return juce::String (v, decimals) + suffix; };
    }

    Fmt signedNum (int decimals, juce::String suffix = {})
    {
        return [decimals, suffix] (double v)
        {
            return juce::String (v > 0.0 ? "+" : "") + juce::String (v, decimals) + suffix;
        };
    }

    Fmt percent()
    {
        return [] (double v) { return juce::String ((int) std::round (v * 100.0)) + " %"; };
    }

    Fmt hertz()
    {
        return [] (double v)
        {
            return v >= 1000.0 ? juce::String (v / 1000.0, 2) + " kHz"
                               : juce::String ((int) std::round (v)) + " Hz";
        };
    }

    Fmt seconds()
    {
        return [] (double v)
        {
            return v < 1.0 ? juce::String ((int) std::round (v * 1000.0)) + " ms"
                           : juce::String (v, 2) + " s";
        };
    }

    void layoutGrid (juce::Rectangle<int> area, const std::vector<juce::Component*>& items, int cols)
    {
        const int rows = ((int) items.size() + cols - 1) / cols;
        const int cw = area.getWidth() / cols;
        const int ch = area.getHeight() / rows;

        for (size_t i = 0; i < items.size(); ++i)
            items[i]->setBounds (area.getX() + (int) (i % (size_t) cols) * cw,
                                 area.getY() + (int) (i / (size_t) cols) * ch,
                                 cw, ch);
    }

    void placeRow (juce::Rectangle<int> area, const std::vector<juce::Component*>& comps,
                   const std::vector<float>& units, int gap)
    {
        float sum = 0.0f;
        for (auto u : units) sum += u;

        const float usable = (float) (area.getWidth() - gap * ((int) comps.size() - 1));

        for (size_t i = 0; i < comps.size(); ++i)
        {
            const int w = (i == comps.size() - 1) ? area.getWidth()
                                                  : juce::roundToInt (usable * units[i] / sum);
            comps[i]->setBounds (area.removeFromLeft (w));
            area.removeFromLeft (gap);
        }
    }
}

//==============================================================================
KnobControl* AnalogSynthEditor::addKnob (Section& s, const char* id, const char* name, juce::Colour c, Fmt fmt)
{
    knobs.push_back (std::make_unique<KnobControl> (proc.apvts, id, name, c, fmt));
    auto* k = knobs.back().get();
    s.addAndMakeVisible (*k);
    return k;
}

ChoiceControl* AnalogSynthEditor::addChoice (Section& s, const char* id, const char* name)
{
    choices.push_back (std::make_unique<ChoiceControl> (proc.apvts, id, name));
    auto* c = choices.back().get();
    s.addAndMakeVisible (*c);
    return c;
}

//==============================================================================
AnalogSynthEditor::AnalogSynthEditor (AnalogSynthProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p), meter (p.outputLevel), curveView (p.apvts)
{
    setLookAndFeel (&laf);

    for (auto* s : { &sOsc, &sFilter, &sLfo, &sAmp, &sFlt, &sChorus, &sDelay, &sReverb, &sInfl })
        addAndMakeVisible (*s);

    using namespace Theme;

    //---------------------------------------------------------------- Oscillators
    {
        auto* w1 = addChoice (sOsc, ID::osc1Wave, "Osc 1");
        auto* w2 = addChoice (sOsc, ID::osc2Wave, "Osc 2");
        std::vector<juce::Component*> k {
            addKnob (sOsc, ID::osc2Semi,   "Semi",    amber, num (0, " st")),
            addKnob (sOsc, ID::osc2Fine,   "Fine",    amber, num (1, " ct")),
            addKnob (sOsc, ID::oscMix,     "Mix",     amber, percent()),
            addKnob (sOsc, ID::pulseWidth, "Pulse",   amber, percent()),
            addKnob (sOsc, ID::subLevel,   "Sub",     amber, percent()),
            addKnob (sOsc, ID::noiseLevel, "Noise",   amber, percent()),
            addKnob (sOsc, ID::drift,      "Drift",   amber, percent()),
            addKnob (sOsc, ID::master,     "Level",   amber, num (1, " dB")) };

        sOsc.onLayout = [w1, w2, k] (juce::Rectangle<int> c)
        {
            auto top = c.removeFromTop (46);
            w1->setBounds (top.removeFromLeft (top.getWidth() / 2));
            w2->setBounds (top);
            layoutGrid (c, k, 4);
        };
    }

    //---------------------------------------------------------------- Filter
    {
        std::vector<juce::Component*> k {
            addKnob (sFilter, ID::cutoff,     "Cutoff",  amber, hertz()),
            addKnob (sFilter, ID::resonance,  "Reson.",  amber, percent()),
            addKnob (sFilter, ID::filtEnvAmt, "Env amt", amber, signedNum (1, " oct")),
            addKnob (sFilter, ID::keyTrack,   "Key trk", amber, percent()),
            addKnob (sFilter, ID::filtDrive,  "Drive",   amber, percent()) };

        sFilter.onLayout = [k] (juce::Rectangle<int> c) { layoutGrid (c, k, 3); };
    }

    //---------------------------------------------------------------- LFO
    {
        std::vector<juce::Component*> k {
            addKnob (sLfo, ID::lfoRate,   "Rate",   amber, num (2, " Hz")),
            addKnob (sLfo, ID::lfoPitch,  "Pitch",  amber, num (2, " st")),
            addKnob (sLfo, ID::lfoCutoff, "Cutoff", amber, num (2, " oct")) };

        sLfo.onLayout = [k] (juce::Rectangle<int> c) { layoutGrid (c, k, 2); };
    }

    //---------------------------------------------------------------- Envelopes
    {
        std::vector<juce::Component*> k {
            addKnob (sAmp, ID::ampA, "Attack",  sky, seconds()),
            addKnob (sAmp, ID::ampD, "Decay",   sky, seconds()),
            addKnob (sAmp, ID::ampS, "Sustain", sky, percent()),
            addKnob (sAmp, ID::ampR, "Release", sky, seconds()) };
        sAmp.onLayout = [k] (juce::Rectangle<int> c) { layoutGrid (c, k, 4); };

        std::vector<juce::Component*> f {
            addKnob (sFlt, ID::fltA, "Attack",  sky, seconds()),
            addKnob (sFlt, ID::fltD, "Decay",   sky, seconds()),
            addKnob (sFlt, ID::fltS, "Sustain", sky, percent()),
            addKnob (sFlt, ID::fltR, "Release", sky, seconds()) };
        sFlt.onLayout = [f] (juce::Rectangle<int> c) { layoutGrid (c, f, 4); };
    }

    //---------------------------------------------------------------- Effects
    {
        std::vector<juce::Component*> k {
            addKnob (sChorus, ID::chorusMix,   "Mix",   violet, percent()),
            addKnob (sChorus, ID::chorusRate,  "Rate",  violet, num (2, " Hz")),
            addKnob (sChorus, ID::chorusDepth, "Depth", violet, percent()) };
        sChorus.onLayout = [k] (juce::Rectangle<int> c) { layoutGrid (c, k, 3); };

        std::vector<juce::Component*> d {
            addKnob (sDelay, ID::delayTime, "Time",     violet, num (0, " ms")),
            addKnob (sDelay, ID::delayFb,   "Feedback", violet, percent()),
            addKnob (sDelay, ID::delayMix,  "Mix",      violet, percent()) };
        sDelay.onLayout = [d] (juce::Rectangle<int> c) { layoutGrid (c, d, 3); };

        std::vector<juce::Component*> r {
            addKnob (sReverb, ID::reverbSize, "Size",    violet, percent()),
            addKnob (sReverb, ID::reverbDamp, "Damping", violet, percent()),
            addKnob (sReverb, ID::reverbMix,  "Mix",     violet, percent()) };
        sReverb.onLayout = [r] (juce::Rectangle<int> c) { layoutGrid (c, r, 3); };
    }

    //---------------------------------------------------------------- Inflator
    {
        std::vector<juce::Component*> k {
            addKnob (sInfl, ID::inflIn,     "Input",  teal, signedNum (1, " dB")),
            addKnob (sInfl, ID::inflEffect, "Effect", teal, num (0, " %")),
            addKnob (sInfl, ID::inflCurve,  "Curve",  teal, signedNum (0, " %")),
            addKnob (sInfl, ID::inflOut,    "Output", teal, num (1, " dB")) };

        inflOn.setButtonText ("ON");
        inflOn.setColour (juce::ToggleButton::tickColourId, teal);
        inflClip.setButtonText ("CLIP");
        inflClip.setColour (juce::ToggleButton::tickColourId, teal);

        sInfl.addAndMakeVisible (inflOn);
        sInfl.addAndMakeVisible (inflClip);
        sInfl.addAndMakeVisible (meter);
        sInfl.addAndMakeVisible (curveView);

        inflOnAtt   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, ID::inflOn, inflOn);
        inflClipAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, ID::inflClip, inflClip);

        sInfl.onLayout = [this, k] (juce::Rectangle<int> c)
        {
            layoutGrid (c.removeFromLeft (c.getWidth() * 4 / 7), k, 4);
            meter.setBounds (c.removeFromRight (16).reduced (1, 6));
            auto toggles = c.removeFromRight (64).withSizeKeepingCentre (60, 58);
            inflOn.setBounds (toggles.removeFromTop (26));
            toggles.removeFromTop (6);
            inflClip.setBounds (toggles.removeFromTop (26));
            curveView.setBounds (c.reduced (6));
        };
    }

    //---------------------------------------------------------------- Presets bar
    {
        const auto& list = Presets::getAll();
        juce::String lastCategory;

        for (size_t i = 0; i < list.size(); ++i)
        {
            if (lastCategory != list[i].category)
            {
                lastCategory = list[i].category;
                presetBox.addSectionHeading (lastCategory);
            }
            presetBox.addItem (list[i].name, (int) i + 1);
        }

        presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
        presetBox.onChange = [this]
        {
            const int id = presetBox.getSelectedId();
            if (id > 0)
                proc.setCurrentProgram (id - 1);
        };

        prevBtn.setButtonText ("<");
        nextBtn.setButtonText (">");
        prevBtn.onClick = [this] { stepPreset (-1); };
        nextBtn.onClick = [this] { stepPreset (+1); };

        addAndMakeVisible (presetBox);
        addAndMakeVisible (prevBtn);
        addAndMakeVisible (nextBtn);
    }

    setResizable (true, true);
    setResizeLimits (900, 610, 1560, 1056);
    getConstrainer()->setFixedAspectRatio (1040.0 / 704.0);
    setSize (1040, 704);

    startTimerHz (30);
}

AnalogSynthEditor::~AnalogSynthEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void AnalogSynthEditor::stepPreset (int delta)
{
    const int n = proc.getNumPrograms();
    if (n <= 0)
        return;

    const int next = (proc.getCurrentProgram() + delta + n) % n;
    proc.setCurrentProgram (next);
    presetBox.setSelectedId (next + 1, juce::dontSendNotification);
}

void AnalogSynthEditor::timerCallback()
{
    meter.repaint();
    curveView.repaint();
}

void AnalogSynthEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::bg);

    g.setFont (Theme::font (22.0f, true));
    g.setColour (Theme::text);
    g.drawText ("ANALOG", 20, 14, 100, 30, juce::Justification::centredLeft);
    g.setColour (Theme::amber);
    g.drawText ("SYNTH", 112, 14, 100, 30, juce::Justification::centredLeft);
}

void AnalogSynthEditor::resized()
{
    auto r = getLocalBounds().reduced (14);

    auto header = r.removeFromTop (48);
    auto bar = header.withSizeKeepingCentre (380, 30);
    prevBtn.setBounds (bar.removeFromLeft (34));
    nextBtn.setBounds (bar.removeFromRight (34));
    presetBox.setBounds (bar.reduced (6, 0));

    r.removeFromTop (8);

    const int gap = 10;
    const int usable = r.getHeight() - 2 * gap;
    const int h1 = juce::roundToInt (usable * 0.38f);
    const int h2 = juce::roundToInt (usable * 0.26f);

    auto row1 = r.removeFromTop (h1);
    r.removeFromTop (gap);
    auto row2 = r.removeFromTop (h2);
    r.removeFromTop (gap);
    auto row3 = r;

    placeRow (row1, { &sOsc, &sFilter, &sLfo },        { 4.0f, 3.0f, 2.0f }, gap);
    placeRow (row2, { &sAmp, &sFlt, &sChorus },        { 4.0f, 4.0f, 3.0f }, gap);
    placeRow (row3, { &sDelay, &sReverb, &sInfl },     { 3.0f, 3.0f, 7.0f }, gap);
}
