#pragma once
#include <JuceHeader.h>

// Simple analog-flavoured ping-pong delay (darkening feedback path)
class StereoDelay
{
public:
    void prepare (double sr)
    {
        sampleRate = sr;
        buffer.setSize (2, (int) (sr * 2.0) + 8);
        buffer.clear();
        writePos = 0;
        lp[0] = lp[1] = 0.0f;
        timeSm.reset (sr, 0.08);
        timeSm.setCurrentAndTargetValue (0.35f);
    }

    void process (juce::AudioBuffer<float>& b, float timeSec, float feedback, float mix)
    {
        timeSm.setTargetValue (timeSec);

        const int size = buffer.getNumSamples();
        const int nc   = juce::jmin (2, b.getNumChannels());
        const int n    = b.getNumSamples();

        float* w0 = buffer.getWritePointer (0);
        float* w1 = buffer.getWritePointer (1);
        float* o0 = b.getWritePointer (0);
        float* o1 = nc > 1 ? b.getWritePointer (1) : nullptr;

        for (int i = 0; i < n; ++i)
        {
            const float dly = juce::jmax (1.0f, timeSm.getNextValue() * (float) sampleRate);
            float rp = (float) writePos - dly;
            while (rp < 0.0f) rp += (float) size;

            const int i0 = (int) rp;
            const int i1 = (i0 + 1) % size;
            const float fr = rp - (float) i0;

            const float d0 = w0[i0] + fr * (w0[i1] - w0[i0]);
            const float d1 = w1[i0] + fr * (w1[i1] - w1[i0]);

            const float in0 = o0[i];
            const float in1 = o1 != nullptr ? o1[i] : in0;
            const float mono = 0.5f * (in0 + in1);

            // ping-pong: left echo feeds right and vice versa, darkened each pass
            lp[0] += 0.4f * (feedback * d1 - lp[0]);
            lp[1] += 0.4f * (feedback * d0 - lp[1]);

            w0[writePos] = mono + lp[0];
            w1[writePos] = lp[1];

            o0[i] = in0 + mix * d0;
            if (o1 != nullptr)
                o1[i] = in1 + mix * d1;

            writePos = (writePos + 1) % size;
        }
    }

private:
    juce::AudioBuffer<float> buffer;
    juce::SmoothedValue<float> timeSm;
    double sampleRate = 44100.0;
    int writePos = 0;
    float lp[2] { 0.0f, 0.0f };
};
