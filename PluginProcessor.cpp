#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Un preset = tous les reglages du synthe (les valeurs par defaut sont celles d'un son "Init")
    struct Preset
    {
        const char* name = "Init";
        int waveA = 0, waveB = 0, unison = 3, arpOn = 0, arpPattern = 0, arpRate = 3, arpOct = 2;
        float semiB = 0, detune = 12, width = 60, mixB = 50, cutoff = 4000, res = 20, envAmt = 30, fltDecay = 600,
              attack = 20, decay = 300, sustain = 70, release = 400, lfoRate = 4, lfoFilter = 0, lfoPitch = 0,
              chorus = 30, delayTime = 375, delayMix = 0, revSize = 40, revMix = 20, arpGate = 70;
    };

    // Formes d'onde : 0 Saw, 1 Square, 2 Triangle, 3 Sine
    const std::vector<Preset>& presets()
    {
        static const std::vector<Preset> list = []
        {
            std::vector<Preset> v;
            { Preset p; p.name = "Warm Pad"; p.detune = 14; p.width = 70; p.cutoff = 1800; p.res = 12; p.envAmt = 15; p.fltDecay = 1800; p.attack = 900; p.decay = 800; p.sustain = 80; p.release = 1800; p.chorus = 45; p.revSize = 65; p.revMix = 30; v.push_back (p); }
            { Preset p; p.name = "Dark Pad"; p.waveB = 1; p.semiB = -12; p.detune = 10; p.cutoff = 650; p.res = 20; p.envAmt = 20; p.fltDecay = 2500; p.attack = 1500; p.decay = 1000; p.sustain = 85; p.release = 2500; p.lfoRate = 0.2f; p.lfoFilter = 15; p.chorus = 30; p.revSize = 75; p.revMix = 35; v.push_back (p); }
            { Preset p; p.name = "Glass Pad"; p.waveA = 2; p.waveB = 3; p.semiB = 12; p.detune = 6; p.unison = 2; p.width = 80; p.mixB = 40; p.cutoff = 9000; p.res = 8; p.envAmt = 0; p.attack = 600; p.decay = 600; p.sustain = 75; p.release = 2200; p.chorus = 50; p.delayTime = 450; p.delayMix = 25; p.revSize = 65; p.revMix = 35; v.push_back (p); }
            { Preset p; p.name = "String Ensemble"; p.detune = 22; p.unison = 5; p.width = 90; p.cutoff = 3200; p.res = 10; p.envAmt = 10; p.fltDecay = 1200; p.attack = 450; p.decay = 600; p.sustain = 85; p.release = 900; p.lfoRate = 5.5f; p.lfoPitch = 8; p.chorus = 60; p.revSize = 45; p.revMix = 20; v.push_back (p); }
            { Preset p; p.name = "Airy Pad"; p.waveA = 1; p.waveB = 2; p.semiB = 12; p.detune = 9; p.width = 85; p.mixB = 35; p.cutoff = 2400; p.res = 18; p.envAmt = 20; p.fltDecay = 1500; p.attack = 1200; p.decay = 900; p.sustain = 80; p.release = 2000; p.lfoRate = 0.35f; p.lfoFilter = 25; p.chorus = 40; p.revSize = 70; p.revMix = 40; v.push_back (p); }
            { Preset p; p.name = "Evolving Pad"; p.semiB = 7; p.detune = 16; p.unison = 4; p.width = 90; p.cutoff = 1500; p.res = 30; p.envAmt = 25; p.fltDecay = 3000; p.attack = 2500; p.decay = 1500; p.sustain = 85; p.release = 3500; p.lfoRate = 0.15f; p.lfoFilter = 60; p.chorus = 50; p.delayTime = 500; p.delayMix = 20; p.revSize = 80; p.revMix = 40; v.push_back (p); }
            { Preset p; p.name = "Sub Pad"; p.waveA = 3; p.waveB = 2; p.semiB = -12; p.detune = 4; p.unison = 2; p.width = 40; p.cutoff = 600; p.res = 5; p.envAmt = 0; p.attack = 800; p.decay = 500; p.sustain = 90; p.release = 1500; p.chorus = 20; p.revSize = 50; p.revMix = 20; v.push_back (p); }
            { Preset p; p.name = "Super Saw Pad"; p.detune = 40; p.unison = 5; p.width = 100; p.cutoff = 6500; p.res = 10; p.envAmt = 10; p.fltDecay = 800; p.attack = 300; p.decay = 500; p.sustain = 85; p.release = 1200; p.chorus = 55; p.revSize = 55; p.revMix = 28; v.push_back (p); }
            { Preset p; p.name = "Ambient Wash"; p.waveB = 2; p.semiB = 12; p.detune = 18; p.unison = 4; p.width = 100; p.mixB = 45; p.cutoff = 3000; p.res = 15; p.attack = 3000; p.decay = 2000; p.sustain = 80; p.release = 6000; p.lfoRate = 0.1f; p.lfoFilter = 30; p.chorus = 50; p.delayTime = 600; p.delayMix = 40; p.revSize = 92; p.revMix = 55; v.push_back (p); }
            { Preset p; p.name = "Retro Pad"; p.waveA = 1; p.detune = 8; p.unison = 2; p.cutoff = 2200; p.res = 25; p.envAmt = 40; p.fltDecay = 800; p.attack = 250; p.decay = 600; p.sustain = 75; p.release = 900; p.chorus = 75; p.revSize = 35; p.revMix = 18; v.push_back (p); }
            { Preset p; p.name = "Arp Pluck"; p.waveB = 1; p.detune = 10; p.unison = 2; p.cutoff = 1400; p.res = 30; p.envAmt = 60; p.fltDecay = 250; p.attack = 1; p.decay = 250; p.sustain = 0; p.release = 150; p.chorus = 25; p.delayTime = 375; p.delayMix = 25; p.revSize = 35; p.revMix = 15; p.arpOn = 1; p.arpPattern = 0; p.arpRate = 3; p.arpOct = 2; p.arpGate = 60; v.push_back (p); }
            { Preset p; p.name = "Arp Pulse"; p.waveA = 1; p.detune = 7; p.unison = 2; p.cutoff = 2500; p.res = 25; p.envAmt = 45; p.fltDecay = 400; p.attack = 2; p.decay = 300; p.sustain = 30; p.release = 200; p.chorus = 40; p.delayTime = 300; p.delayMix = 30; p.revSize = 40; p.revMix = 20; p.arpOn = 1; p.arpPattern = 2; p.arpRate = 1; p.arpOct = 3; p.arpGate = 55; v.push_back (p); }
            { Preset p; p.name = "Lead Synth"; p.waveB = 1; p.detune = 14; p.unison = 2; p.width = 50; p.cutoff = 3500; p.res = 25; p.envAmt = 30; p.fltDecay = 400; p.attack = 5; p.decay = 200; p.sustain = 70; p.release = 250; p.lfoRate = 5.5f; p.lfoPitch = 18; p.chorus = 25; p.delayTime = 330; p.delayMix = 20; p.revSize = 30; p.revMix = 15; v.push_back (p); }
            { Preset p; p.name = "Bass Synth"; p.waveB = 1; p.semiB = -12; p.detune = 6; p.unison = 1; p.width = 20; p.cutoff = 500; p.res = 35; p.envAmt = 55; p.fltDecay = 300; p.attack = 2; p.decay = 200; p.sustain = 60; p.release = 120; p.chorus = 0; p.revSize = 20; p.revMix = 5; v.push_back (p); }
            return v;
        }();
        return list;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout WarlordSynthProcessor::createParameterLayout()
{
    using Range = juce::NormalisableRange<float>;
    using PID = juce::ParameterID;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addF = [&layout] (const char* id, const char* name, float lo, float hi, float step, float def, float skew = 1.0f)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat> (PID { id, 1 }, name, Range (lo, hi, step, skew), def));
    };
    auto addC = [&layout] (const char* id, const char* name, juce::StringArray items, int def)
    {
        layout.add (std::make_unique<juce::AudioParameterChoice> (PID { id, 1 }, name, items, def));
    };
    auto addB = [&layout] (const char* id, const char* name, bool def)
    {
        layout.add (std::make_unique<juce::AudioParameterBool> (PID { id, 1 }, name, def));
    };

    const juce::StringArray waves { "Saw", "Square", "Triangle", "Sine" };
    addC (ids::waveA, "Wave A", waves, 0);
    addC (ids::waveB, "Wave B", waves, 0);
    addF (ids::semiB, "Semitone B", -24.0f, 24.0f, 1.0f, 0.0f);
    addF (ids::detune, "Detune", 0.0f, 100.0f, 0.1f, 12.0f);
    addF (ids::unison, "Unison", 1.0f, 5.0f, 1.0f, 3.0f);
    addF (ids::width, "Width", 0.0f, 100.0f, 0.1f, 60.0f);
    addF (ids::mixB, "Osc Mix", 0.0f, 100.0f, 0.1f, 50.0f);

    addF (ids::cutoff, "Cutoff", 20.0f, 20000.0f, 1.0f, 4000.0f, 0.3f);
    addF (ids::res, "Resonance", 0.0f, 100.0f, 0.1f, 20.0f);
    addF (ids::envAmt, "Filter Env Amount", -100.0f, 100.0f, 0.1f, 30.0f);
    addF (ids::fltDecay, "Filter Decay", 20.0f, 5000.0f, 1.0f, 600.0f, 0.4f);

    addF (ids::attack, "Attack", 1.0f, 8000.0f, 1.0f, 20.0f, 0.35f);
    addF (ids::decay, "Decay", 1.0f, 5000.0f, 1.0f, 300.0f, 0.4f);
    addF (ids::sustain, "Sustain", 0.0f, 100.0f, 0.1f, 70.0f);
    addF (ids::release, "Release", 5.0f, 10000.0f, 1.0f, 400.0f, 0.35f);

    addF (ids::lfoRate, "LFO Rate", 0.05f, 20.0f, 0.01f, 4.0f, 0.4f);
    addF (ids::lfoFilter, "LFO to Filter", 0.0f, 100.0f, 0.1f, 0.0f);
    addF (ids::lfoPitch, "LFO to Pitch", 0.0f, 100.0f, 0.1f, 0.0f);

    addF (ids::chorus, "Chorus", 0.0f, 100.0f, 0.1f, 30.0f);
    addF (ids::delayTime, "Delay Time", 50.0f, 1000.0f, 1.0f, 375.0f);
    addF (ids::delayMix, "Delay Mix", 0.0f, 100.0f, 0.1f, 0.0f);
    addF (ids::revSize, "Reverb Size", 0.0f, 100.0f, 0.1f, 40.0f);
    addF (ids::revMix, "Reverb Mix", 0.0f, 100.0f, 0.1f, 20.0f);
    addF (ids::master, "Master", -30.0f, 6.0f, 0.1f, -6.0f);

    addB (ids::arpOn, "Arp On", false);
    addB (ids::arpLatch, "Arp Latch", false);
    addC (ids::arpPattern, "Arp Pattern", { "Up", "Down", "Up-Down", "Random", "As Played" }, 0);
    addC (ids::arpRate, "Arp Rate", { "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" }, 3);
    addF (ids::arpOct, "Arp Octaves", 1.0f, 4.0f, 1.0f, 2.0f);
    addF (ids::arpGate, "Arp Gate", 10.0f, 100.0f, 1.0f, 70.0f);
    return layout;
}

WarlordSynthProcessor::WarlordSynthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createParameterLayout())
{
    for (int i = 0; i < 16; ++i)
        synth.addVoice (new SynthVoice (params));
    synth.addSound (new SynthSound());
    synth.setNoteStealingEnabled (true);
}

bool WarlordSynthProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void WarlordSynthProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    synth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            voice->prepare (sampleRate, samplesPerBlock);

    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    chorus.prepare (spec);
    chorus.setRate (0.6f);
    chorus.setDepth (0.4f);
    chorus.setCentreDelay (8.0f);
    chorus.setFeedback (0.0f);
    chorus.reset();

    delayLine.prepare (spec);
    delayLine.reset();
    reverb.setSampleRate (sampleRate);
    reverb.reset();

    masterS.reset (sampleRate, 0.02);
    masterS.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (v (ids::master)));
    delayTimeS.reset (sampleRate, 0.2);
    delayTimeS.setCurrentAndTargetValue (v (ids::delayTime) * 0.001f * (float) sampleRate);

    arp.reset();
    wasArpOn = false;
    keyboardState.reset();
}

void WarlordSynthProcessor::updateParams()
{
    params.waveA = (int) v (ids::waveA);
    params.waveB = (int) v (ids::waveB);
    params.unison = juce::roundToInt (v (ids::unison));
    params.semiB = v (ids::semiB);
    params.detune = v (ids::detune);
    params.width = v (ids::width) * 0.01f;
    params.mixB = v (ids::mixB) * 0.01f;
    params.cutoff = v (ids::cutoff);
    params.q = 0.707f + v (ids::res) * 0.01f * 9.0f;
    params.envAmt = v (ids::envAmt) * 0.01f;
    params.fltDecay = v (ids::fltDecay) * 0.001f;
    params.attack = v (ids::attack) * 0.001f;
    params.decay = v (ids::decay) * 0.001f;
    params.sustain = v (ids::sustain) * 0.01f;
    params.release = v (ids::release) * 0.001f;
    params.lfoRate = v (ids::lfoRate);
    params.lfoFilter = v (ids::lfoFilter) * 0.01f;
    params.lfoPitch = v (ids::lfoPitch);
}

void WarlordSynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();

    keyboardState.processNextMidiBuffer (midi, 0, n, true);   // notes du clavier a l'ecran
    updateParams();

    // ---- Arpegiateur ----
    const bool arpOn = v (ids::arpOn) > 0.5f;
    if (arpOn != wasArpOn)
    {
        synth.allNotesOff (0, true);
        arp.reset();
        wasArpOn = arpOn;
    }
    if (arpOn)
    {
        double bpm = 120.0, ppq = 0.0;
        bool playing = false;
        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
            {
                bpm = pos->getBpm().orFallback (120.0);
                if (auto q = pos->getPpqPosition())
                {
                    ppq = *q;
                    playing = pos->getIsPlaying();
                }
            }

        ArpSettings s;
        s.pattern = (int) v (ids::arpPattern);
        s.rate = (int) v (ids::arpRate);
        s.octaves = juce::roundToInt (v (ids::arpOct));
        s.gate = v (ids::arpGate) * 0.01f;
        s.latch = v (ids::arpLatch) > 0.5f;
        arp.process (midi, n, sr, bpm, playing, ppq, s);
        arpStep.store (arp.getDisplayStep());
    }
    else
    {
        arpStep.store (-1);
    }

    // ---- Synthe ----
    synth.renderNextBlock (buffer, midi, 0, n);

    // ---- Effets : chorus, delay, reverb ----
    const float chorusMix = v (ids::chorus) * 0.01f;
    if (chorusMix > 0.001f)
    {
        chorus.setMix (chorusMix);
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        chorus.process (ctx);
    }

    const float dMix = v (ids::delayMix) * 0.01f;
    delayTimeS.setTargetValue (v (ids::delayTime) * 0.001f * (float) sr);
    for (int i = 0; i < n; ++i)
    {
        delayLine.setDelay (delayTimeS.getNextValue());
        for (int ch = 0; ch < 2; ++ch)
        {
            float* d = buffer.getWritePointer (ch);
            const float del = delayLine.popSample (ch);
            delayLine.pushSample (ch, d[i] + del * 0.35f);    // rebouclage fixe de 35 %
            d[i] += del * dMix;
        }
    }

    const float rMix = v (ids::revMix) * 0.01f;
    juce::Reverb::Parameters rp;
    rp.roomSize = v (ids::revSize) * 0.01f;
    rp.damping = 0.45f;
    rp.wetLevel = 0.6f * rMix;
    rp.dryLevel = 1.0f - 0.3f * rMix;
    rp.width = 1.0f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
    reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), n);

    // ---- Volume general + ecretage doux de securite ----
    masterS.setTargetValue (juce::Decibels::decibelsToGain (v (ids::master)));
    for (int i = 0; i < n; ++i)
    {
        const float g = masterS.getNextValue();
        for (int ch = 0; ch < 2; ++ch)
        {
            float* d = buffer.getWritePointer (ch);
            d[i] = std::tanh (d[i] * g);
        }
    }
}

int WarlordSynthProcessor::getNumPrograms() { return (int) presets().size(); }

const juce::String WarlordSynthProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, getNumPrograms()) ? presets()[(size_t) index].name : "";
}

void WarlordSynthProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, getNumPrograms()))
        return;

    currentProgram = index;
    const auto& p = presets()[(size_t) index];

    auto set = [this] (const char* id, float value)
    {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    };

    set (ids::waveA, (float) p.waveA);       set (ids::waveB, (float) p.waveB);
    set (ids::semiB, p.semiB);               set (ids::detune, p.detune);
    set (ids::unison, (float) p.unison);     set (ids::width, p.width);
    set (ids::mixB, p.mixB);                 set (ids::cutoff, p.cutoff);
    set (ids::res, p.res);                   set (ids::envAmt, p.envAmt);
    set (ids::fltDecay, p.fltDecay);         set (ids::attack, p.attack);
    set (ids::decay, p.decay);               set (ids::sustain, p.sustain);
    set (ids::release, p.release);           set (ids::lfoRate, p.lfoRate);
    set (ids::lfoFilter, p.lfoFilter);       set (ids::lfoPitch, p.lfoPitch);
    set (ids::chorus, p.chorus);             set (ids::delayTime, p.delayTime);
    set (ids::delayMix, p.delayMix);         set (ids::revSize, p.revSize);
    set (ids::revMix, p.revMix);
    set (ids::arpOn, (float) p.arpOn);       set (ids::arpPattern, (float) p.arpPattern);
    set (ids::arpRate, (float) p.arpRate);   set (ids::arpOct, (float) p.arpOct);
    set (ids::arpGate, p.arpGate);           set (ids::arpLatch, 0.0f);
}

juce::AudioProcessorEditor* WarlordSynthProcessor::createEditor() { return new WarlordSynthEditor (*this); }

void WarlordSynthProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void WarlordSynthProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            currentProgram = xml->getIntAttribute ("program", 0);
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new WarlordSynthProcessor(); }
