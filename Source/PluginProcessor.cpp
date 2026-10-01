#include "PluginProcessor.h"
#include "PluginEditor.h"

AnalogSynthProcessor::AnalogSynthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    synth.addSound (new SynthSound());
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new SynthVoice (apvts));
}

juce::AudioProcessorValueTreeState::ParameterLayout AnalogSynthProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto addF = [&params] (const char* id, const char* name, float lo, float hi, float step, float def, float skew = 1.0f)
    {
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, juce::NormalisableRange<float> (lo, hi, step, skew), def));
    };
    auto addC = [&params] (const char* id, const char* name, const juce::StringArray& choices, int def)
    {
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id, 1 }, name, choices, def));
    };
    auto addB = [&params] (const char* id, const char* name, bool def)
    {
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id, 1 }, name, def));
    };

    const juce::StringArray waves { "Saw", "Square", "Triangle", "Sine" };

    // Oscillators
    addC (ID::osc1Wave, "Osc1 Wave", waves, 0);
    addC (ID::osc2Wave, "Osc2 Wave", waves, 0);
    addF (ID::osc2Semi,   "Osc2 Semitones", -24.f, 24.f, 1.f, 0.f);
    addF (ID::osc2Fine,   "Osc2 Fine", -100.f, 100.f, 0.1f, 7.f);
    addF (ID::oscMix,     "Osc Mix", 0.f, 1.f, 0.001f, 0.5f);
    addF (ID::pulseWidth, "Pulse Width", 0.05f, 0.95f, 0.001f, 0.5f);
    addF (ID::subLevel,   "Sub Level", 0.f, 1.f, 0.001f, 0.f);
    addF (ID::noiseLevel, "Noise Level", 0.f, 1.f, 0.001f, 0.f);
    addF (ID::drift,      "Analog Drift", 0.f, 1.f, 0.001f, 0.3f);
    addF (ID::master,     "Level", -36.f, 6.f, 0.1f, -6.f);

    // Filter
    addF (ID::cutoff,     "Cutoff", 20.f, 20000.f, 0.f, 1500.f, 0.25f);
    addF (ID::resonance,  "Resonance", 0.f, 1.f, 0.001f, 0.3f);
    addF (ID::filtEnvAmt, "Filter Env Amount", -6.f, 6.f, 0.01f, 2.5f);
    addF (ID::keyTrack,   "Key Tracking", 0.f, 1.f, 0.001f, 0.5f);
    addF (ID::filtDrive,  "Filter Drive", 0.f, 1.f, 0.001f, 0.f);

    // Envelopes
    addF (ID::ampA, "Amp Attack",  0.001f, 8.f, 0.f, 0.005f, 0.35f);
    addF (ID::ampD, "Amp Decay",   0.001f, 8.f, 0.f, 0.3f,   0.35f);
    addF (ID::ampS, "Amp Sustain", 0.f, 1.f, 0.001f, 0.7f);
    addF (ID::ampR, "Amp Release", 0.001f, 8.f, 0.f, 0.25f,  0.35f);
    addF (ID::fltA, "Filter Attack",  0.001f, 8.f, 0.f, 0.005f, 0.35f);
    addF (ID::fltD, "Filter Decay",   0.001f, 8.f, 0.f, 0.4f,   0.35f);
    addF (ID::fltS, "Filter Sustain", 0.f, 1.f, 0.001f, 0.2f);
    addF (ID::fltR, "Filter Release", 0.001f, 8.f, 0.f, 0.3f,   0.35f);

    // LFO
    addF (ID::lfoRate,   "LFO Rate",   0.05f, 20.f, 0.f, 5.f, 0.4f);
    addF (ID::lfoPitch,  "LFO Pitch",  0.f, 2.f, 0.001f, 0.f);
    addF (ID::lfoCutoff, "LFO Cutoff", 0.f, 4.f, 0.001f, 0.f);

    // Effects
    addF (ID::chorusMix,   "Chorus Mix",   0.f, 1.f, 0.001f, 0.f);
    addF (ID::chorusRate,  "Chorus Rate",  0.1f, 5.f, 0.f, 0.8f, 0.5f);
    addF (ID::chorusDepth, "Chorus Depth", 0.f, 1.f, 0.001f, 0.4f);
    addF (ID::delayTime,   "Delay Time",   20.f, 1500.f, 0.f, 350.f, 0.5f);
    addF (ID::delayFb,     "Delay Feedback", 0.f, 0.95f, 0.001f, 0.35f);
    addF (ID::delayMix,    "Delay Mix",    0.f, 1.f, 0.001f, 0.f);
    addF (ID::reverbSize,  "Reverb Size",  0.f, 1.f, 0.001f, 0.5f);
    addF (ID::reverbDamp,  "Reverb Damping", 0.f, 1.f, 0.001f, 0.5f);
    addF (ID::reverbMix,   "Reverb Mix",   0.f, 1.f, 0.001f, 0.f);

    // Inflator
    addB (ID::inflOn,     "Inflator On", true);
    addF (ID::inflIn,     "Inflator Input",  -12.f, 12.f, 0.1f, 0.f);
    addF (ID::inflEffect, "Inflator Effect", 0.f, 100.f, 0.1f, 0.f);
    addF (ID::inflCurve,  "Inflator Curve",  -50.f, 50.f, 0.1f, 0.f);
    addF (ID::inflOut,    "Inflator Output", -24.f, 0.f, 0.1f, 0.f);
    addB (ID::inflClip,   "Inflator Clip", true);

    return { params.begin(), params.end() };
}

//==============================================================================
void AnalogSynthProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    chorus.prepare (spec);
    chorus.reset();
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback (0.0f);

    reverb.setSampleRate (sampleRate);
    reverb.reset();

    delay.prepare (sampleRate);
    inflator.prepare (sampleRate);
}

bool AnalogSynthProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void AnalogSynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int n = buffer.getNumSamples();
    buffer.clear();
    synth.renderNextBlock (buffer, midi, 0, n);

    // Chorus
    const float cMix = p (ID::chorusMix);
    if (cMix > 0.001f)
    {
        chorus.setRate (p (ID::chorusRate));
        chorus.setDepth (p (ID::chorusDepth));
        chorus.setMix (cMix);
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        chorus.process (ctx);
    }

    // Delay (always runs so the buffer stays clean)
    delay.process (buffer, p (ID::delayTime) * 0.001f, p (ID::delayFb), p (ID::delayMix));

    // Reverb
    const float rMix = p (ID::reverbMix);
    if (rMix > 0.001f)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize   = p (ID::reverbSize);
        rp.damping    = p (ID::reverbDamp);
        rp.wetLevel   = rMix * 0.6f;
        rp.dryLevel   = 1.0f - rMix * 0.3f;
        rp.width      = 1.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        if (buffer.getNumChannels() >= 2)
            reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), n);
        else
            reverb.processMono (buffer.getWritePointer (0), n);
    }

    // Inflator
    if (p (ID::inflOn) > 0.5f)
    {
        inflator.setParams (p (ID::inflIn),
                            p (ID::inflEffect) * 0.01f,
                            p (ID::inflCurve) / 50.0f,
                            p (ID::inflOut),
                            p (ID::inflClip) > 0.5f);
        inflator.process (buffer);
    }

    // Output level for the meter
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, n));
    outputLevel.store (juce::jmax (peak, outputLevel.load() * 0.85f));
}

//==============================================================================
int AnalogSynthProcessor::getNumPrograms()
{
    return (int) Presets::getAll().size();
}

const juce::String AnalogSynthProcessor::getProgramName (int index)
{
    const auto& list = Presets::getAll();
    if (index >= 0 && index < (int) list.size())
        return list[(size_t) index].name;
    return {};
}

void AnalogSynthProcessor::setCurrentProgram (int index)
{
    const auto& list = Presets::getAll();
    if (index < 0 || index >= (int) list.size())
        return;

    currentPreset = index;

    for (auto* prm : getParameters())
        prm->setValueNotifyingHost (prm->getDefaultValue());

    for (const auto& kv : list[(size_t) index].values)
        if (auto* prm = apvts.getParameter (kv.first))
            prm->setValueNotifyingHost (prm->convertTo0to1 (kv.second));
}

juce::AudioProcessorEditor* AnalogSynthProcessor::createEditor()
{
    return new AnalogSynthEditor (*this);
}

void AnalogSynthProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("preset", currentPreset, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, dest);
}

void AnalogSynthProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto state = juce::ValueTree::fromXml (*xml);
            currentPreset = juce::jlimit (0, getNumPrograms() - 1, (int) state.getProperty ("preset", 0));
            apvts.replaceState (state);
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogSynthProcessor();
}
