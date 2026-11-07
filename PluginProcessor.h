#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include "SynthEngine.h"
#include "Arpeggiator.h"

namespace ids
{
    inline constexpr const char* waveA = "waveA";       inline constexpr const char* waveB = "waveB";
    inline constexpr const char* semiB = "semiB";       inline constexpr const char* detune = "detune";
    inline constexpr const char* unison = "unison";     inline constexpr const char* width = "width";
    inline constexpr const char* mixB = "mixB";
    inline constexpr const char* cutoff = "cutoff";     inline constexpr const char* res = "res";
    inline constexpr const char* envAmt = "envAmt";     inline constexpr const char* fltDecay = "fltDecay";
    inline constexpr const char* attack = "attack";     inline constexpr const char* decay = "decay";
    inline constexpr const char* sustain = "sustain";   inline constexpr const char* release = "release";
    inline constexpr const char* lfoRate = "lfoRate";   inline constexpr const char* lfoFilter = "lfoFilter";
    inline constexpr const char* lfoPitch = "lfoPitch";
    inline constexpr const char* chorus = "chorus";     inline constexpr const char* delayTime = "delayTime";
    inline constexpr const char* delayMix = "delayMix"; inline constexpr const char* revSize = "revSize";
    inline constexpr const char* revMix = "revMix";     inline constexpr const char* master = "master";
    inline constexpr const char* arpOn = "arpOn";       inline constexpr const char* arpLatch = "arpLatch";
    inline constexpr const char* arpPattern = "arpPattern"; inline constexpr const char* arpRate = "arpRate";
    inline constexpr const char* arpOct = "arpOct";     inline constexpr const char* arpGate = "arpGate";
}

class WarlordSynthProcessor : public juce::AudioProcessor
{
public:
    WarlordSynthProcessor();
    ~WarlordSynthProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Warlord Synth"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    // Presets (pads, arpeges, lead, basse)
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;   // clavier a l'ecran
    std::atomic<int> arpStep { -1 };         // pas courant de l'arpegiateur (pour l'affichage)

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float v (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    void updateParams();

    juce::Synthesiser synth;
    SynthParams params;
    Arpeggiator arp;
    bool wasArpOn = false;

    juce::dsp::Chorus<float> chorus;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 200000 };
    juce::Reverb reverb;
    juce::SmoothedValue<float> masterS, delayTimeS;
    double sr = 44100.0;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WarlordSynthProcessor)
};
