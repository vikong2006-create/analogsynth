#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

//==============================================================================
// Rounded panel with a title; children are positioned by onLayout
class Section : public juce::Component
{
public:
    Section (const juce::String& t, juce::Colour c) : title (t), colour (c) {}

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (Theme::panel);
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (Theme::stroke);
        g.drawRoundedRectangle (r, 10.0f, 1.0f);

        g.setColour (colour);
        g.fillEllipse (16.0f, 15.0f, 6.0f, 6.0f);
        g.setColour (Theme::dim);
        g.setFont (Theme::font (11.0f, true));
        g.drawText (title.toUpperCase(), 28, 8, getWidth() - 40, 20, juce::Justification::centredLeft);
    }

    void resized() override
    {
        if (onLayout)
            onLayout (getLocalBounds().withTrimmedTop (30).reduced (8, 4));
    }

    std::function<void (juce::Rectangle<int>)> onLayout;

private:
    juce::String title;
    juce::Colour colour;
};

//==============================================================================
class KnobControl : public juce::Component
{
public:
    KnobControl (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                 const juce::String& name, juce::Colour colour,
                 std::function<juce::String (double)> format)
        : label (name.toUpperCase())
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 16);
        slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f, true);
        slider.setVelocityBasedMode (false);
        addAndMakeVisible (slider);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);

        if (format)
        {
            slider.textFromValueFunction = format;
            slider.updateText();
        }

        if (auto* prm = apvts.getParameter (paramId))
            slider.setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
    }

    void resized() override
    {
        auto r = getLocalBounds();
        nameArea = r.removeFromBottom (14);
        slider.setBounds (r);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (Theme::dim);
        g.setFont (Theme::font (10.0f, true));
        g.drawText (label, nameArea, juce::Justification::centred);
    }

private:
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::String label;
    juce::Rectangle<int> nameArea;
};

//==============================================================================
class ChoiceControl : public juce::Component
{
public:
    ChoiceControl (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& name)
        : label (name.toUpperCase())
    {
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramId)))
            combo.addItemList (choice->choices, 1);

        addAndMakeVisible (combo);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramId, combo);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6, 0);
        nameArea = r.removeFromTop (14);
        combo.setBounds (r.removeFromTop (26));
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (Theme::dim);
        g.setFont (Theme::font (10.0f, true));
        g.drawText (label, nameArea, juce::Justification::centredLeft);
    }

private:
    juce::ComboBox combo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    juce::String label;
    juce::Rectangle<int> nameArea;
};

//==============================================================================
// Vertical output level meter
class LevelMeter : public juce::Component
{
public:
    explicit LevelMeter (const std::atomic<float>& levelRef) : level (levelRef) {}

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (Theme::panel2);
        g.fillRoundedRectangle (r, 3.0f);

        const float db = juce::Decibels::gainToDecibels (level.load(), -60.0f);
        const float prop = juce::jlimit (0.0f, 1.0f, juce::jmap (db, -60.0f, 0.0f, 0.0f, 1.0f));

        auto bar = r.reduced (2.0f);
        const float h = bar.getHeight() * prop;
        auto fill = bar.removeFromBottom (h);

        g.setColour (prop > 0.95f ? juce::Colour (0xffff5a5a) : Theme::teal);
        g.fillRoundedRectangle (fill, 2.0f);
    }

private:
    const std::atomic<float>& level;
};

//==============================================================================
// Live display of the Inflator transfer curve
class CurveView : public juce::Component
{
public:
    explicit CurveView (juce::AudioProcessorValueTreeState& s) : apvts (s) {}

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (Theme::panel2);
        g.fillRoundedRectangle (r, 6.0f);

        auto plot = r.reduced (10.0f);

        // grid
        g.setColour (Theme::stroke);
        g.drawLine (plot.getX(), plot.getCentreY(), plot.getRight(), plot.getCentreY(), 1.0f);
        g.drawLine (plot.getCentreX(), plot.getY(), plot.getCentreX(), plot.getBottom(), 1.0f);
        g.drawRect (plot, 1.0f);

        // identity line
        g.setColour (Theme::dim.withAlpha (0.45f));
        g.drawLine (plot.getX(), plot.getBottom(), plot.getRight(), plot.getY(), 1.0f);

        const bool on   = get (ID::inflOn) > 0.5f;
        const float inG = juce::Decibels::decibelsToGain (get (ID::inflIn));
        const float eff = get (ID::inflEffect) * 0.01f;
        const float crv = get (ID::inflCurve) / 50.0f;
        const float outG = juce::Decibels::decibelsToGain (get (ID::inflOut));
        const bool clip = get (ID::inflClip) > 0.5f;

        juce::Path path;
        const int N = 160;
        for (int i = 0; i <= N; ++i)
        {
            const float x = -1.0f + 2.0f * (float) i / (float) N;
            float y = x;

            if (on)
            {
                const float xin = x * inG;
                const float wet = InflatorMath::shape (xin, crv, clip);
                y = (xin + eff * (wet - xin)) * outG;
            }

            y = juce::jlimit (-1.0f, 1.0f, y);
            const float px = plot.getX() + (x + 1.0f) * 0.5f * plot.getWidth();
            const float py = plot.getBottom() - (y + 1.0f) * 0.5f * plot.getHeight();

            if (i == 0) path.startNewSubPath (px, py);
            else        path.lineTo (px, py);
        }

        g.setColour (on ? Theme::teal : Theme::dim);
        g.strokePath (path, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

private:
    float get (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    juce::AudioProcessorValueTreeState& apvts;
};

//==============================================================================
class AnalogSynthEditor : public juce::AudioProcessorEditor,
                          private juce::Timer
{
public:
    explicit AnalogSynthEditor (AnalogSynthProcessor&);
    ~AnalogSynthEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using Fmt = std::function<juce::String (double)>;

    KnobControl*   addKnob (Section& s, const char* id, const char* name, juce::Colour c, Fmt fmt);
    ChoiceControl* addChoice (Section& s, const char* id, const char* name);
    void timerCallback() override;
    void stepPreset (int delta);

    AnalogSynthProcessor& proc;
    MinimalLookAndFeel laf;

    Section sOsc    { "Oscillators", Theme::amber };
    Section sFilter { "Filter",      Theme::amber };
    Section sLfo    { "LFO",         Theme::amber };
    Section sAmp    { "Amp envelope",    Theme::sky };
    Section sFlt    { "Filter envelope", Theme::sky };
    Section sChorus { "Chorus", Theme::violet };
    Section sDelay  { "Delay",  Theme::violet };
    Section sReverb { "Reverb", Theme::violet };
    Section sInfl   { "Inflator", Theme::teal };

    std::vector<std::unique_ptr<KnobControl>> knobs;
    std::vector<std::unique_ptr<ChoiceControl>> choices;

    LevelMeter meter;
    CurveView curveView;
    juce::ToggleButton inflOn, inflClip;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> inflOnAtt, inflClipAtt;

    juce::ComboBox presetBox;
    juce::TextButton prevBtn, nextBtn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthEditor)
};
