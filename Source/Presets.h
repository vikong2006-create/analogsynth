#pragma once
#include <JuceHeader.h>
#include "Synth.h"

// Every parameter not listed in a preset is reset to its default value.
// Wave: 0 = Saw, 1 = Square, 2 = Triangle, 3 = Sine
struct Preset
{
    const char* category;
    const char* name;
    std::vector<std::pair<const char*, float>> values;
};

namespace Presets
{
    inline const std::vector<Preset>& getAll()
    {
        using namespace ID;

        static const std::vector<Preset> list =
        {
            { "Init", "Init Analog", {} },

            //================================================================ BASS
            { "Bass", "Moog Sub Bass", {
                { osc1Wave, 0.f }, { osc2Wave, 1.f }, { osc2Semi, -12.f }, { osc2Fine, 3.f },
                { oscMix, 0.4f }, { subLevel, 0.6f },
                { cutoff, 280.f }, { resonance, 0.25f }, { filtEnvAmt, 3.0f }, { keyTrack, 0.3f }, { filtDrive, 0.4f },
                { ampA, 0.003f }, { ampD, 0.25f }, { ampS, 0.8f }, { ampR, 0.12f },
                { fltD, 0.22f }, { fltS, 0.1f }, { fltR, 0.15f },
                { inflEffect, 40.f }, { inflCurve, 10.f } } },

            { "Bass", "Acid Squelch", {
                { osc1Wave, 0.f }, { oscMix, 0.0f },
                { cutoff, 350.f }, { resonance, 0.85f }, { filtEnvAmt, 4.0f }, { keyTrack, 0.6f }, { filtDrive, 0.5f },
                { ampA, 0.002f }, { ampD, 0.15f }, { ampS, 0.6f }, { ampR, 0.08f },
                { fltA, 0.001f }, { fltD, 0.18f }, { fltS, 0.0f }, { fltR, 0.1f },
                { delayMix, 0.15f }, { delayTime, 375.f }, { delayFb, 0.4f },
                { inflEffect, 50.f } } },

            { "Bass", "Wobble Bass", {
                { osc1Wave, 0.f }, { osc2Wave, 1.f }, { osc2Semi, -12.f }, { osc2Fine, 5.f },
                { oscMix, 0.5f }, { subLevel, 0.4f },
                { cutoff, 400.f }, { resonance, 0.55f }, { filtEnvAmt, 0.0f }, { filtDrive, 0.4f },
                { lfoRate, 2.5f }, { lfoCutoff, 2.5f },
                { ampS, 1.0f }, { ampR, 0.15f },
                { inflEffect, 60.f }, { inflCurve, 15.f } } },

            { "Bass", "Fat Reese", {
                { osc1Wave, 0.f }, { osc2Wave, 0.f }, { osc2Fine, 25.f }, { oscMix, 0.5f }, { subLevel, 0.3f },
                { cutoff, 700.f }, { resonance, 0.2f }, { filtEnvAmt, 1.0f }, { filtDrive, 0.3f },
                { ampA, 0.01f }, { ampD, 0.4f }, { ampS, 0.9f }, { ampR, 0.3f },
                { fltS, 0.5f },
                { chorusMix, 0.3f }, { chorusRate, 0.4f }, { chorusDepth, 0.6f },
                { inflEffect, 55.f } } },

            //================================================================ LEAD
            { "Lead", "Mini Lead", {
                { osc1Wave, 0.f }, { osc2Wave, 0.f }, { osc2Fine, 8.f }, { oscMix, 0.5f },
                { cutoff, 2200.f }, { resonance, 0.35f }, { filtEnvAmt, 1.8f },
                { ampA, 0.005f }, { ampD, 0.2f }, { ampS, 0.8f }, { ampR, 0.25f },
                { fltD, 0.3f }, { fltS, 0.4f },
                { lfoRate, 5.5f }, { lfoPitch, 0.1f }, { drift, 0.4f },
                { delayMix, 0.2f }, { delayTime, 320.f }, { delayFb, 0.35f },
                { reverbMix, 0.15f }, { reverbSize, 0.4f },
                { inflEffect, 45.f } } },

            { "Lead", "Square Solo", {
                { osc1Wave, 1.f }, { osc2Wave, 1.f }, { osc2Semi, 7.f }, { osc2Fine, 4.f }, { oscMix, 0.35f },
                { pulseWidth, 0.35f },
                { cutoff, 3000.f }, { resonance, 0.2f }, { filtEnvAmt, 1.0f },
                { ampS, 0.85f }, { ampR, 0.2f },
                { lfoRate, 5.0f }, { lfoPitch, 0.15f },
                { delayMix, 0.15f }, { reverbMix, 0.2f } } },

            { "Lead", "Brass Stab", {
                { osc1Wave, 0.f }, { osc2Wave, 0.f }, { osc2Fine, 10.f }, { oscMix, 0.5f },
                { cutoff, 600.f }, { resonance, 0.15f }, { filtEnvAmt, 3.5f }, { filtDrive, 0.35f },
                { ampA, 0.03f }, { ampD, 0.3f }, { ampS, 0.8f }, { ampR, 0.15f },
                { fltA, 0.06f }, { fltD, 0.25f }, { fltS, 0.5f }, { fltR, 0.2f },
                { chorusMix, 0.25f },
                { inflEffect, 60.f }, { inflCurve, 20.f } } },

            { "Lead", "Hollow Lead", {
                { osc1Wave, 2.f }, { osc2Wave, 1.f }, { osc2Semi, 12.f }, { osc2Fine, 6.f }, { oscMix, 0.3f },
                { pulseWidth, 0.25f },
                { cutoff, 1800.f }, { resonance, 0.4f }, { filtEnvAmt, 1.5f },
                { ampA, 0.01f }, { ampS, 0.85f }, { ampR, 0.3f },
                { lfoRate, 4.5f }, { lfoPitch, 0.12f },
                { delayMix, 0.25f }, { delayTime, 450.f }, { delayFb, 0.4f },
                { reverbMix, 0.25f } } },

            //================================================================ PAD
            { "Pad", "Warm Analog Pad", {
                { osc1Wave, 0.f }, { osc2Wave, 0.f }, { osc2Fine, 12.f }, { oscMix, 0.5f },
                { cutoff, 900.f }, { resonance, 0.2f }, { filtEnvAmt, 1.2f },
                { ampA, 1.2f }, { ampD, 1.0f }, { ampS, 0.8f }, { ampR, 1.8f },
                { fltA, 1.5f }, { fltD, 2.0f }, { fltS, 0.5f }, { fltR, 2.0f },
                { lfoRate, 0.25f }, { lfoCutoff, 0.4f }, { drift, 0.6f },
                { chorusMix, 0.45f }, { chorusRate, 0.3f }, { chorusDepth, 0.6f },
                { reverbMix, 0.35f }, { reverbSize, 0.7f } } },

            { "Pad", "Juno Strings", {
                { osc1Wave, 0.f }, { osc2Wave, 0.f }, { osc2Fine, 14.f }, { oscMix, 0.5f }, { subLevel, 0.2f },
                { cutoff, 4500.f }, { resonance, 0.1f }, { filtEnvAmt, 0.0f },
                { ampA, 0.5f }, { ampD, 0.8f }, { ampS, 0.9f }, { ampR, 1.2f },
                { drift, 0.5f },
                { chorusMix, 0.6f }, { chorusRate, 0.6f }, { chorusDepth, 0.7f },
                { reverbMix, 0.25f }, { reverbSize, 0.6f } } },

            { "Pad", "Dark Evolving", {
                { osc1Wave, 0.f }, { osc2Wave, 1.f }, { osc2Semi, -12.f }, { osc2Fine, 9.f }, { oscMix, 0.5f },
                { noiseLevel, 0.08f },
                { cutoff, 500.f }, { resonance, 0.4f }, { filtEnvAmt, 2.0f },
                { ampA, 2.5f }, { ampD, 2.0f }, { ampS, 0.85f }, { ampR, 3.0f },
                { fltA, 3.0f }, { fltD, 3.0f }, { fltS, 0.4f }, { fltR, 3.0f },
                { lfoRate, 0.15f }, { lfoCutoff, 1.5f },
                { delayMix, 0.25f }, { delayTime, 550.f }, { delayFb, 0.5f },
                { reverbMix, 0.5f }, { reverbSize, 0.85f }, { reverbDamp, 0.6f },
                { inflEffect, 40.f }, { inflCurve, 30.f } } },

            //================================================================ KEYS
            { "Keys", "Electric Keys", {
                { osc1Wave, 2.f }, { osc2Wave, 3.f }, { osc2Semi, 12.f }, { osc2Fine, 2.f }, { oscMix, 0.3f },
                { cutoff, 2500.f }, { resonance, 0.1f }, { filtEnvAmt, 1.5f }, { filtDrive, 0.1f },
                { ampA, 0.002f }, { ampD, 0.9f }, { ampS, 0.3f }, { ampR, 0.4f },
                { fltD, 0.6f }, { fltS, 0.2f },
                { chorusMix, 0.35f },
                { reverbMix, 0.15f } } },

            { "Keys", "Soft Organ", {
                { osc1Wave, 3.f }, { osc2Wave, 3.f }, { osc2Semi, 12.f }, { osc2Fine, 0.f }, { oscMix, 0.5f },
                { subLevel, 0.5f },
                { cutoff, 6000.f }, { resonance, 0.0f }, { filtEnvAmt, 0.0f },
                { ampA, 0.01f }, { ampD, 0.1f }, { ampS, 1.0f }, { ampR, 0.08f },
                { lfoRate, 6.0f }, { lfoPitch, 0.05f },
                { chorusMix, 0.4f }, { chorusRate, 3.0f }, { chorusDepth, 0.3f } } },

            //================================================================ PLUCK
            { "Pluck", "Classic Pluck", {
                { osc1Wave, 0.f }, { osc2Wave, 1.f }, { osc2Fine, 6.f }, { oscMix, 0.5f },
                { cutoff, 500.f }, { resonance, 0.3f }, { filtEnvAmt, 3.5f },
                { ampA, 0.001f }, { ampD, 0.35f }, { ampS, 0.0f }, { ampR, 0.25f },
                { fltD, 0.25f }, { fltS, 0.0f },
                { delayMix, 0.2f }, { delayTime, 375.f }, { delayFb, 0.4f },
                { reverbMix, 0.2f } } },

            { "Pluck", "Glass Bell", {
                { osc1Wave, 3.f }, { osc2Wave, 3.f }, { osc2Semi, 19.f }, { osc2Fine, -5.f }, { oscMix, 0.45f },
                { cutoff, 8000.f }, { resonance, 0.1f }, { filtEnvAmt, 0.0f },
                { ampA, 0.001f }, { ampD, 1.5f }, { ampS, 0.0f }, { ampR, 1.5f },
                { reverbMix, 0.4f }, { reverbSize, 0.7f } } },

            { "Pluck", "Marimba", {
                { osc1Wave, 2.f }, { osc2Wave, 3.f }, { osc2Semi, 12.f }, { oscMix, 0.25f },
                { cutoff, 1800.f }, { resonance, 0.15f }, { filtEnvAmt, 1.5f },
                { ampA, 0.001f }, { ampD, 0.5f }, { ampS, 0.0f }, { ampR, 0.2f },
                { fltD, 0.3f }, { fltS, 0.0f },
                { reverbMix, 0.2f } } },

            //================================================================ FX
            { "FX", "Laser Zap", {
                { osc1Wave, 0.f }, { osc2Wave, 1.f }, { osc2Semi, 7.f }, { oscMix, 0.5f },
                { cutoff, 8000.f }, { resonance, 0.6f }, { filtEnvAmt, -4.0f },
                { ampA, 0.001f }, { ampD, 0.3f }, { ampS, 0.0f }, { ampR, 0.1f },
                { fltA, 0.001f }, { fltD, 0.25f }, { fltS, 0.0f }, { fltR, 0.1f },
                { lfoRate, 18.f }, { lfoPitch, 2.0f },
                { delayMix, 0.2f }, { delayTime, 250.f } } },

            { "FX", "Noise Sweep", {
                { osc1Wave, 0.f }, { oscMix, 0.0f }, { noiseLevel, 1.0f },
                { cutoff, 300.f }, { resonance, 0.6f }, { filtEnvAmt, 4.0f },
                { ampA, 1.5f }, { ampD, 1.0f }, { ampS, 0.7f }, { ampR, 2.0f },
                { fltA, 3.0f }, { fltD, 2.0f }, { fltS, 0.0f }, { fltR, 1.0f },
                { reverbMix, 0.4f }, { reverbSize, 0.8f } } },

            { "FX", "Sci-Fi Drone", {
                { osc1Wave, 0.f }, { osc2Wave, 1.f }, { osc2Semi, -12.f }, { osc2Fine, 40.f }, { oscMix, 0.5f },
                { subLevel, 0.5f }, { noiseLevel, 0.1f },
                { cutoff, 700.f }, { resonance, 0.5f }, { filtEnvAmt, 0.0f },
                { lfoRate, 0.3f }, { lfoCutoff, 2.0f }, { lfoPitch, 0.3f },
                { ampA, 2.0f }, { ampS, 1.0f }, { ampR, 3.0f },
                { delayMix, 0.35f }, { delayTime, 600.f }, { delayFb, 0.6f },
                { reverbMix, 0.5f }, { reverbSize, 0.9f },
                { inflEffect, 70.f }, { inflCurve, 25.f } } },

            //================================================================ ARP
            { "Arp", "Sequence Pluck", {
                { osc1Wave, 0.f }, { oscMix, 0.0f },
                { cutoff, 600.f }, { resonance, 0.45f }, { filtEnvAmt, 3.5f }, { filtDrive, 0.3f },
                { ampA, 0.001f }, { ampD, 0.18f }, { ampS, 0.0f }, { ampR, 0.1f },
                { fltD, 0.15f }, { fltS, 0.0f },
                { delayMix, 0.3f }, { delayTime, 375.f }, { delayFb, 0.45f },
                { inflEffect, 35.f } } },

            { "Arp", "Inflated Saw", {
                { osc1Wave, 0.f }, { osc2Wave, 0.f }, { osc2Fine, 12.f }, { oscMix, 0.5f },
                { cutoff, 1400.f }, { resonance, 0.3f }, { filtEnvAmt, 2.5f },
                { ampA, 0.002f }, { ampD, 0.25f }, { ampS, 0.0f }, { ampR, 0.15f },
                { fltD, 0.2f }, { fltS, 0.0f },
                { delayMix, 0.2f }, { delayTime, 250.f }, { delayFb, 0.35f },
                { inflIn, 6.f }, { inflEffect, 100.f }, { inflCurve, 35.f }, { inflOut, -4.f } } },
        };

        return list;
    }
}
