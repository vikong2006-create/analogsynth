#pragma once
#include <JuceHeader.h>
#include <cmath>

//==============================================================================
// Inflator-style waveshaper: raises perceived loudness by adding harmonics.
// Not the Sonnox algorithm, but the same idea: a polynomial transfer curve
// with a "curve" control that adds asymmetry (even harmonics).
namespace InflatorMath
{
    // x: input sample, curve: -1..1, clip: hard clip at 0 dBFS
    inline float shape (float x, float curve, bool clip)
    {
        const float a = std::abs (x);

        if (a >= 1.0f)
            return clip ? (x > 0.0f ? 1.0f : -1.0f) : x;

        const float x2 = x * x;
        const float odd  = 1.5f * x - 0.5f * x2 * x;      // soft saturation
        const float even = 0.5f * curve * (x2 - x2 * x2);  // asymmetry (even harmonics)
        return odd + even;
    }
}

class InflatorFx
{
public:
    void prepare (double sampleRate)
    {
        dcR = 1.0f - juce::MathConstants<float>::twoPi * 10.0f / (float) sampleRate;
        inG.reset (sampleRate, 0.03);
        outG.reset (sampleRate, 0.03);
        eff.reset (sampleRate, 0.03);
        crv.reset (sampleRate, 0.03);
        first = true;
        for (int i = 0; i < 2; ++i) { xPrev[i] = 0.0f; yPrev[i] = 0.0f; }
    }

    void setParams (float inDb, float effect01, float curveMinus1To1, float outDb, bool clipOn)
    {
        const float ig = juce::Decibels::decibelsToGain (inDb);
        const float og = juce::Decibels::decibelsToGain (outDb);

        if (first)
        {
            inG.setCurrentAndTargetValue (ig);
            outG.setCurrentAndTargetValue (og);
            eff.setCurrentAndTargetValue (effect01);
            crv.setCurrentAndTargetValue (curveMinus1To1);
            first = false;
        }
        else
        {
            inG.setTargetValue (ig);
            outG.setTargetValue (og);
            eff.setTargetValue (effect01);
            crv.setTargetValue (curveMinus1To1);
        }
        clip = clipOn;
    }

    void process (juce::AudioBuffer<float>& b)
    {
        const int n  = b.getNumSamples();
        const int nc = juce::jmin (b.getNumChannels(), 2);

        for (int i = 0; i < n; ++i)
        {
            const float g = inG.getNextValue();
            const float e = eff.getNextValue();
            const float c = crv.getNextValue();
            const float o = outG.getNextValue();

            for (int ch = 0; ch < nc; ++ch)
            {
                float* d = b.getWritePointer (ch);
                const float x   = d[i] * g;
                const float wet = InflatorMath::shape (x, c, clip);
                const float y   = x + e * (wet - x);

                // DC blocker (even harmonics can introduce DC)
                const float dc = y - xPrev[ch] + dcR * yPrev[ch];
                xPrev[ch] = y;
                yPrev[ch] = dc;

                d[i] = dc * o;
            }
        }
    }

private:
    juce::SmoothedValue<float> inG, outG, eff, crv;
    bool clip = true;
    bool first = true;
    float dcR = 0.999f;
    float xPrev[2] { 0, 0 };
    float yPrev[2] { 0, 0 };
};
