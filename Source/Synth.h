#pragma once
#include <JuceHeader.h>
#include <cmath>

//==============================================================================
// Parameter IDs
namespace ID
{
    inline constexpr const char* osc1Wave   = "osc1Wave";
    inline constexpr const char* osc2Wave   = "osc2Wave";
    inline constexpr const char* osc2Semi   = "osc2Semi";
    inline constexpr const char* osc2Fine   = "osc2Fine";
    inline constexpr const char* oscMix     = "oscMix";
    inline constexpr const char* pulseWidth = "pulseWidth";
    inline constexpr const char* subLevel   = "subLevel";
    inline constexpr const char* noiseLevel = "noiseLevel";

    inline constexpr const char* cutoff     = "cutoff";
    inline constexpr const char* resonance  = "resonance";
    inline constexpr const char* filtEnvAmt = "filtEnvAmt";
    inline constexpr const char* keyTrack   = "keyTrack";

    inline constexpr const char* ampA = "ampA";
    inline constexpr const char* ampD = "ampD";
    inline constexpr const char* ampS = "ampS";
    inline constexpr const char* ampR = "ampR";
    inline constexpr const char* fltA = "fltA";
    inline constexpr const char* fltD = "fltD";
    inline constexpr const char* fltS = "fltS";
    inline constexpr const char* fltR = "fltR";

    inline constexpr const char* lfoRate     = "lfoRate";
    inline constexpr const char* lfoPitch    = "lfoPitch";
    inline constexpr const char* lfoCutoff   = "lfoCutoff";

    inline constexpr const char* drift  = "drift";
    inline constexpr const char* master = "master";
    inline constexpr const char* filtDrive = "filtDrive";

    // Effects
    inline constexpr const char* chorusMix   = "chorusMix";
    inline constexpr const char* chorusRate  = "chorusRate";
    inline constexpr const char* chorusDepth = "chorusDepth";
    inline constexpr const char* delayTime   = "delayTime";
    inline constexpr const char* delayFb     = "delayFb";
    inline constexpr const char* delayMix    = "delayMix";
    inline constexpr const char* reverbSize  = "reverbSize";
    inline constexpr const char* reverbDamp  = "reverbDamp";
    inline constexpr const char* reverbMix   = "reverbMix";

    // Inflator-style harmonic enhancer
    inline constexpr const char* inflOn     = "inflOn";
    inline constexpr const char* inflIn     = "inflIn";
    inline constexpr const char* inflEffect = "inflEffect";
    inline constexpr const char* inflCurve  = "inflCurve";
    inline constexpr const char* inflOut    = "inflOut";
    inline constexpr const char* inflClip   = "inflClip";
}

//==============================================================================
struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

//==============================================================================
// Band-limited (PolyBLEP) oscillator
struct Oscillator
{
    double phase = 0.0;

    static float polyBlep (double t, double dt)
    {
        if (t < dt)       { t /= dt;           return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
        return 0.0f;
    }

    // wave: 0 saw, 1 square/pulse, 2 triangle, 3 sine
    float next (int wave, double freq, double sampleRate, float pw)
    {
        const double dt = juce::jlimit (1.0e-6, 0.49, freq / sampleRate);
        float out = 0.0f;

        switch (wave)
        {
            case 0:
                out = (float) (2.0 * phase - 1.0);
                out -= polyBlep (phase, dt);
                break;
            case 1:
            {
                out = phase < pw ? 1.0f : -1.0f;
                out += polyBlep (phase, dt);
                double t2 = phase - pw; if (t2 < 0.0) t2 += 1.0;
                out -= polyBlep (t2, dt);
                break;
            }
            case 2:
                out = (float) (2.0 * std::abs (2.0 * phase - 1.0) - 1.0);
                break;
            default:
                out = std::sin ((float) (phase * juce::MathConstants<double>::twoPi));
                break;
        }

        phase += dt;
        if (phase >= 1.0) phase -= 1.0;
        return out;
    }
};

//==============================================================================
// Moog-style 4-pole resonant low-pass ladder (tanh-saturated, 2x oversampled)
struct LadderFilter
{
    float s[4] { 0, 0, 0, 0 };

    void reset() { for (auto& v : s) v = 0.0f; }

    float process (float in, float cutoffHz, float resonance, float sampleRate)
    {
        const float fs2 = sampleRate * 2.0f;
        const float fc  = juce::jlimit (20.0f, 0.45f * fs2, cutoffHz);
        const float g   = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * fc / fs2);
        const float k   = resonance * 3.9f;

        for (int n = 0; n < 2; ++n)
        {
            const float x = std::tanh (in - k * s[3]);
            s[0] += g * (x    - s[0]);
            s[1] += g * (s[0] - s[1]);
            s[2] += g * (s[1] - s[2]);
            s[3] += g * (s[2] - s[3]);
        }
        return s[3];
    }
};

//==============================================================================
class SynthVoice : public juce::SynthesiserVoice
{
public:
    explicit SynthVoice (juce::AudioProcessorValueTreeState& state) : apvts (state)
    {
        random.setSeedRandomly();
    }

    bool canPlaySound (juce::SynthesiserSound* s) override
    {
        return dynamic_cast<SynthSound*> (s) != nullptr;
    }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int currentPitchWheel) override
    {
        noteNumber = midiNote;
        vel        = velocity;
        baseFreq   = juce::MidiMessage::getMidiNoteInHertz (midiNote);
        pitchWheelMoved (currentPitchWheel);

        // per-voice random "analog" pitch offset (cents)
        driftCents = (random.nextFloat() * 2.0f - 1.0f);
        lfoPhase   = 0.0;
        filter.reset();

        updateParams();
        ampEnv.setSampleRate (getSampleRate());
        filtEnv.setSampleRate (getSampleRate());
        ampEnv.setParameters (ampParams);
        filtEnv.setParameters (fltParams);
        ampEnv.noteOn();
        filtEnv.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            ampEnv.noteOff();
            filtEnv.noteOff();
        }
        else
        {
            ampEnv.reset();
            filtEnv.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved (int newValue) override
    {
        bendSemis = ((float) newValue - 8192.0f) / 8192.0f * 2.0f;
    }

    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        updateParams();
        ampEnv.setParameters (ampParams);
        filtEnv.setParameters (fltParams);

        const double sr = getSampleRate();
        const int numCh = out.getNumChannels();

        for (int i = 0; i < numSamples; ++i)
        {
            const float ae = ampEnv.getNextSample();
            const float fe = filtEnv.getNextSample();

            if (! ampEnv.isActive())
            {
                clearCurrentNote();
                break;
            }

            // LFO
            const float lfo = std::sin ((float) (lfoPhase * juce::MathConstants<double>::twoPi));
            lfoPhase += lfoRate / sr;
            if (lfoPhase >= 1.0) lfoPhase -= 1.0;

            // pitch (semitones)
            const double semis = bendSemis + lfo * lfoPitch + driftCents * driftAmt * 0.12;
            const double f1 = baseFreq * std::exp2 (semis / 12.0);
            const double f2 = f1 * std::exp2 ((osc2Semi + osc2Fine / 100.0) / 12.0);

            // sources
            const float o1 = osc1.next (osc1Wave, f1, sr, pw);
            const float o2 = osc2.next (osc2Wave, f2, sr, pw);
            const float sub = subOsc.next (3, f1 * 0.5, sr, 0.5f);
            const float noise = random.nextFloat() * 2.0f - 1.0f;

            float mixed = o1 * (1.0f - oscMix) + o2 * oscMix;
            mixed += sub * subLevel + noise * noiseLevel;

            // filter cutoff modulation (in octaves)
            const float octaves = filtEnvAmt * fe
                                + lfoCutoffAmt * lfo
                                + keyTrack * (float) (noteNumber - 60) / 12.0f;
            const float fc = cutoff * std::exp2 (octaves);

            float y = filter.process (mixed * driveGain, fc, resonance, (float) sr);
            y *= ae * vel * masterGain;

            for (int ch = 0; ch < numCh; ++ch)
                out.addSample (ch, startSample + i, y);
        }
    }

private:
    float p (const char* id) const { return apvts.getRawParameterValue (id)->load(); }

    void updateParams()
    {
        osc1Wave = (int) p (ID::osc1Wave);
        osc2Wave = (int) p (ID::osc2Wave);
        osc2Semi = p (ID::osc2Semi);
        osc2Fine = p (ID::osc2Fine);
        oscMix   = p (ID::oscMix);
        pw       = p (ID::pulseWidth);
        subLevel = p (ID::subLevel);
        noiseLevel = p (ID::noiseLevel) * 0.5f;

        cutoff     = p (ID::cutoff);
        resonance  = p (ID::resonance);
        filtEnvAmt = p (ID::filtEnvAmt);
        keyTrack   = p (ID::keyTrack);
        driveGain  = 0.5f * (1.0f + p (ID::filtDrive) * 4.0f);

        ampParams = { p (ID::ampA), p (ID::ampD), p (ID::ampS), p (ID::ampR) };
        fltParams = { p (ID::fltA), p (ID::fltD), p (ID::fltS), p (ID::fltR) };

        lfoRate       = p (ID::lfoRate);
        lfoPitch      = p (ID::lfoPitch);
        lfoCutoffAmt  = p (ID::lfoCutoff);

        driftAmt   = p (ID::drift);
        masterGain = juce::Decibels::decibelsToGain (p (ID::master));
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::Random random;

    Oscillator osc1, osc2, subOsc;
    LadderFilter filter;
    juce::ADSR ampEnv, filtEnv;
    juce::ADSR::Parameters ampParams, fltParams;

    int    noteNumber = 60;
    float  vel = 1.0f;
    double baseFreq = 440.0;
    float  bendSemis = 0.0f;
    float  driftCents = 0.0f;
    double lfoPhase = 0.0;

    int   osc1Wave = 0, osc2Wave = 0;
    float osc2Semi = 0, osc2Fine = 0, oscMix = 0.5f, pw = 0.5f;
    float subLevel = 0, noiseLevel = 0;
    float cutoff = 2000, resonance = 0.2f, filtEnvAmt = 2.0f, keyTrack = 0.5f;
    float lfoRate = 5.0f, lfoPitch = 0.0f, lfoCutoffAmt = 0.0f;
    float driftAmt = 0.3f, masterGain = 1.0f, driveGain = 0.5f;
};
