#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

// Reglages lus par toutes les voix (mis a jour par le processeur a chaque bloc)
struct SynthParams
{
    int   waveA = 0, waveB = 0, unison = 3;
    float semiB = 0.0f, detune = 12.0f, width = 0.6f, mixB = 0.5f;
    float cutoff = 4000.0f, q = 0.8f, envAmt = 0.3f, fltDecay = 0.6f;
    float attack = 0.02f, decay = 0.3f, sustain = 0.7f, release = 0.4f;
    float lfoRate = 4.0f, lfoFilter = 0.0f, lfoPitch = 0.0f;
};

struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

namespace Osc
{
    // Correction polyBLEP : limite l'aliasing des formes d'onde a angles vifs
    inline float polyBlep (float t, float dt) noexcept
    {
        if (t < dt)         { t /= dt;                  return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt)  { t = (t - 1.0f) / dt;      return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    // 0 = Saw, 1 = Square, 2 = Triangle, 3 = Sine
    inline float generate (int wave, float phase, float dt) noexcept
    {
        switch (wave)
        {
            case 0:  return 2.0f * phase - 1.0f - polyBlep (phase, dt);
            case 1:
            {
                float p2 = phase + 0.5f;
                if (p2 >= 1.0f) p2 -= 1.0f;
                return (phase < 0.5f ? 1.0f : -1.0f) + polyBlep (phase, dt) - polyBlep (p2, dt);
            }
            case 2:  return 1.0f - 4.0f * std::abs (phase - 0.5f);
            default: return std::sin (juce::MathConstants<float>::twoPi * phase);
        }
    }
}

class SynthVoice : public juce::SynthesiserVoice
{
public:
    static constexpr int maxUnison = 5;

    explicit SynthVoice (const SynthParams& p) : params (p) {}

    void prepare (double sampleRate, int blockSize)
    {
        sr = sampleRate;
        ampEnv.setSampleRate (sampleRate);
        fltEnv.setSampleRate (sampleRate);
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) blockSize, 2 };
        filter.prepare (spec);
        filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    }

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<SynthSound*> (s) != nullptr; }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int) override
    {
        noteFreq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);
        vel = 0.35f + 0.65f * velocity;   // la velocite change le niveau sans jamais couper le son
        for (int u = 0; u < maxUnison; ++u)
        {
            phaseA[u] = rng.nextFloat();
            phaseB[u] = rng.nextFloat();
        }
        lfoPhase = 0.0f;
        ctr = 0;
        filter.reset();
        filter.setResonance (params.q);

        ampEnv.setParameters ({ params.attack, params.decay, params.sustain, params.release });
        fltEnv.setParameters ({ params.attack, params.fltDecay, 0.0f, params.release });
        ampEnv.noteOn();
        fltEnv.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            ampEnv.noteOff();
            fltEnv.noteOff();
        }
        else
        {
            ampEnv.reset();
            fltEnv.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    using juce::SynthesiserVoice::renderNextBlock;

    void renderNextBlock (juce::AudioBuffer<float>& out, int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        const auto& P = params;
        const int uni = juce::jlimit (1, maxUnison, P.unison);
        const float fs = (float) sr;
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float semiRatio = std::exp2 (P.semiB / 12.0f);
        const float gainB = juce::jmin (1.0f, 2.0f * P.mixB);
        const float gainA = juce::jmin (1.0f, 2.0f * (1.0f - P.mixB));
        const float norm  = 0.22f / std::sqrt ((float) uni);
        const float maxCut = juce::jmin (20000.0f, 0.45f * fs);
        filter.setResonance (P.q);

        // Detune et position stereo de chaque voix d'unisson
        float ratio[maxUnison], gl[maxUnison], gr[maxUnison];
        for (int u = 0; u < uni; ++u)
        {
            const float pos = uni == 1 ? 0.0f : ((float) u / (float) (uni - 1)) * 2.0f - 1.0f;   // -1 a +1
            ratio[u] = std::exp2 (pos * P.detune / 1200.0f);
            const float ang = (pos * P.width + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
            gl[u] = std::cos (ang);
            gr[u] = std::sin (ang);
        }

        for (int i = 0; i < numSamples; ++i)
        {
            lfoPhase += P.lfoRate / fs;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
            const float lfo = std::sin (twoPi * lfoPhase);
            const float pitchMul = P.lfoPitch > 0.0f ? std::exp2 (lfo * P.lfoPitch * 0.5f / 1200.0f) : 1.0f;

            float l = 0.0f, r = 0.0f;
            for (int u = 0; u < uni; ++u)
            {
                const float f = noteFreq * ratio[u] * pitchMul;
                const float dtA = f / fs, dtB = f * semiRatio / fs;
                const float s = Osc::generate (P.waveA, phaseA[u], dtA) * gainA
                              + Osc::generate (P.waveB, phaseB[u], dtB) * gainB;
                phaseA[u] += dtA; if (phaseA[u] >= 1.0f) phaseA[u] -= 1.0f;
                phaseB[u] += dtB; if (phaseB[u] >= 1.0f) phaseB[u] -= 1.0f;
                l += s * gl[u];
                r += s * gr[u];
            }

            const float fenv = fltEnv.getNextSample();
            if ((ctr++ & 7) == 0)   // la frequence de coupure est mise a jour tous les 8 echantillons
            {
                const float octaves = P.envAmt * 5.0f * fenv + lfo * P.lfoFilter * 2.0f;
                filter.setCutoffFrequency (juce::jlimit (20.0f, maxCut, P.cutoff * std::exp2 (octaves)));
            }
            l = filter.processSample (0, l);
            r = filter.processSample (1, r);

            const float amp = ampEnv.getNextSample() * vel * norm;
            out.addSample (0, startSample + i, l * amp);
            if (out.getNumChannels() > 1)
                out.addSample (1, startSample + i, r * amp);

            if (! ampEnv.isActive())
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    const SynthParams& params;
    double sr = 44100.0;
    float noteFreq = 440.0f, vel = 1.0f, lfoPhase = 0.0f;
    float phaseA[maxUnison] {}, phaseB[maxUnison] {};
    int ctr = 0;
    juce::ADSR ampEnv, fltEnv;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::Random rng;
};
