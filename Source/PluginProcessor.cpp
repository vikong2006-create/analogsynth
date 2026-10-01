#include "PluginProcessor.h"

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
    using F = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;
    using R = juce::NormalisableRange<float>;

    juce::StringArray waves { "Saw", "Square", "Triangle", "Sine" };
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    // Oscillators
    p.push_back (std::make_unique<C> (juce::ParameterID { ID::osc1Wave, 1 }, "Osc1 Wave", waves, 0));
    p.push_back (std::make_unique<C> (juce::ParameterID { ID::osc2Wave, 1 }, "Osc2 Wave", waves, 0));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::osc2Semi, 1 }, "Osc2 Semitones", R (-24.f, 24.f, 1.f), 0.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::osc2Fine, 1 }, "Osc2 Fine (cents)", R (-100.f, 100.f, 0.1f), 7.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::oscMix, 1 },   "Osc Mix", R (0.f, 1.f, 0.001f), 0.5f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::pulseWidth, 1 }, "Pulse Width", R (0.05f, 0.95f, 0.001f), 0.5f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::subLevel, 1 },   "Sub Level", R (0.f, 1.f, 0.001f), 0.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::noiseLevel, 1 }, "Noise Level", R (0.f, 1.f, 0.001f), 0.f));

    // Filter
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::cutoff, 1 },    "Cutoff", R (20.f, 20000.f, 0.f, 0.25f), 1500.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::resonance, 1 }, "Resonance", R (0.f, 1.f, 0.001f), 0.3f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::filtEnvAmt, 1 }, "Filter Env Amount (oct)", R (-6.f, 6.f, 0.01f), 2.5f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::keyTrack, 1 },   "Key Tracking", R (0.f, 1.f, 0.001f), 0.5f));

    // Envelopes
    auto time = [] { return R (0.001f, 8.f, 0.f, 0.35f); };
    auto sus  = [] { return R (0.f, 1.f, 0.001f); };
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::ampA, 1 }, "Amp Attack",  time(), 0.005f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::ampD, 1 }, "Amp Decay",   time(), 0.3f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::ampS, 1 }, "Amp Sustain", sus(),  0.7f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::ampR, 1 }, "Amp Release", time(), 0.25f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::fltA, 1 }, "Filter Attack",  time(), 0.005f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::fltD, 1 }, "Filter Decay",   time(), 0.4f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::fltS, 1 }, "Filter Sustain", sus(),  0.2f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::fltR, 1 }, "Filter Release", time(), 0.3f));

    // LFO
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::lfoRate, 1 },   "LFO Rate (Hz)", R (0.05f, 20.f, 0.f, 0.4f), 5.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::lfoPitch, 1 },  "LFO > Pitch (semi)", R (0.f, 2.f, 0.001f), 0.f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::lfoCutoff, 1 }, "LFO > Cutoff (oct)", R (0.f, 4.f, 0.001f), 0.f));

    // Global
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::drift, 1 },  "Analog Drift", R (0.f, 1.f, 0.001f), 0.3f));
    p.push_back (std::make_unique<F> (juce::ParameterID { ID::master, 1 }, "Master (dB)", R (-36.f, 6.f, 0.1f), -6.f));

    return { p.begin(), p.end() };
}

void AnalogSynthProcessor::prepareToPlay (double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
}

bool AnalogSynthProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void AnalogSynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* AnalogSynthProcessor::createEditor()
{
    // Auto-generated UI with a slider/combo for every parameter
    return new juce::GenericAudioProcessorEditor (*this);
}

void AnalogSynthProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void AnalogSynthProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogSynthProcessor();
}
