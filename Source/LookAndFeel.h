#pragma once
#include <JuceHeader.h>

namespace Theme
{
    inline const juce::Colour bg     { 0xff0e0f11 };
    inline const juce::Colour panel  { 0xff17181b };
    inline const juce::Colour panel2 { 0xff1f2024 };
    inline const juce::Colour stroke { 0xff2a2c31 };
    inline const juce::Colour text   { 0xffeceef0 };
    inline const juce::Colour dim    { 0xff80838b };
    inline const juce::Colour amber  { 0xffff9a3c };
    inline const juce::Colour sky    { 0xff5aa9ff };
    inline const juce::Colour violet { 0xffa78bfa };
    inline const juce::Colour teal   { 0xff35d0ba };

    inline juce::Font font (float height, bool bold = false)
    {
        juce::Font f (juce::FontOptions (height));
        return bold ? f.boldened() : f;
    }
}

class MinimalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MinimalLookAndFeel()
    {
        using namespace Theme;
        setColour (juce::ComboBox::backgroundColourId, panel2);
        setColour (juce::ComboBox::textColourId, text);
        setColour (juce::ComboBox::outlineColourId, stroke);
        setColour (juce::ComboBox::arrowColourId, dim);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1c1d21));
        setColour (juce::PopupMenu::textColourId, text);
        setColour (juce::PopupMenu::headerTextColourId, dim);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, amber);
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
        setColour (juce::Slider::textBoxTextColourId, text);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::rotarySliderFillColourId, amber);
        setColour (juce::Label::textColourId, text);
        setColour (juce::TextButton::buttonColourId, panel2);
        setColour (juce::TextButton::textColourOffId, text);
        setColour (juce::ToggleButton::tickColourId, amber);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float pos, float startAngle, float endAngle, juce::Slider& slider) override
    {
        using namespace Theme;
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f);
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const float lw = 3.0f;
        const float arcR = radius - lw * 0.5f;
        const float angle = startAngle + pos * (endAngle - startAngle);

        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
        g.setColour (stroke);
        g.strokePath (track, juce::PathStrokeType (lw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (pos > 0.001f)
        {
            juce::Path val;
            val.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
            g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
            g.strokePath (val, juce::PathStrokeType (lw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        const float bodyR = juce::jmax (4.0f, arcR - 7.0f);
        g.setColour (panel2);
        g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.setColour (stroke);
        g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

        juce::Path pointer;
        pointer.addRoundedRectangle (-1.3f, -bodyR + 2.5f, 2.6f, bodyR * 0.45f, 1.2f);
        g.setColour (text);
        g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool) override
    {
        using namespace Theme;
        auto r = b.getLocalBounds().toFloat().reduced (1.0f);
        const bool on = b.getToggleState();

        g.setColour (on ? b.findColour (juce::ToggleButton::tickColourId) : panel2);
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (on ? juce::Colours::black.withAlpha (0.0f) : stroke);
        g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.0f);

        g.setColour (on ? juce::Colours::black : (highlighted ? text : dim));
        g.setFont (font (11.0f, true));
        g.drawText (b.getButtonText(), r.toNearestInt(), juce::Justification::centred);
    }

    juce::Font getLabelFont (juce::Label&) override          { return Theme::font (11.5f); }
    juce::Font getComboBoxFont (juce::ComboBox&) override    { return Theme::font (12.5f); }
    juce::Font getPopupMenuFont() override                   { return Theme::font (13.0f); }
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return Theme::font (15.0f, true); }
};
