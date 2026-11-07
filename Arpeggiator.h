#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

struct ArpSettings
{
    int pattern = 0;      // 0 Up, 1 Down, 2 Up-Down, 3 Random, 4 As Played
    int rate = 3;         // 0 1/4, 1 1/8, 2 1/8T, 3 1/16, 4 1/16T, 5 1/32
    int octaves = 2;
    float gate = 0.7f;    // 0.1 a 1.0
    bool latch = false;
};

// Arpegiateur monophonique : lit les notes tenues et joue une note a la fois, calee sur le tempo.
// Si l'hote joue, les pas sont alignes sur sa grille (position en noires) ; sinon il tourne librement.
class Arpeggiator
{
public:
    void reset()
    {
        held.clear();
        down.clear();
        currentNote = -1;
        countdown = offCountdown = 0.0;
        stepCounter = 0;
        lastStepNumber = -1;
        displayStep = -1;
    }

    int getDisplayStep() const { return displayStep; }

    void process (juce::MidiBuffer& midi, int numSamples, double sampleRate, double bpm,
                  bool hostPlaying, double ppq, const ArpSettings& s)
    {
        juce::MidiBuffer out;

        // 1. Les notes jouees au clavier alimentent la liste ; le reste (molette, CC) passe tel quel
        for (const auto meta : midi)
        {
            const auto msg = meta.getMessage();
            if (msg.isNoteOn())                                   keyDown (msg.getNoteNumber(), (int) msg.getVelocity(), s.latch);
            else if (msg.isNoteOff())                             keyUp (msg.getNoteNumber(), s.latch);
            else if (msg.isAllNotesOff() || msg.isAllSoundOff())  { held.clear(); down.clear(); }
            else                                                  out.addEvent (msg, meta.samplePosition);
        }
        if (! s.latch && down.empty())
            held.clear();

        const double stepBeats = rateBeats (s.rate);
        const double samplesPerBeat = sampleRate * 60.0 / juce::jmax (20.0, bpm);
        const double stepSamples = stepBeats * samplesPerBeat;
        const double gateSamples = juce::jmax (8.0, stepSamples * (double) s.gate);

        // 2. Plus aucune note tenue : on coupe la note en cours
        if (held.empty())
        {
            if (currentNote >= 0)
            {
                out.addEvent (juce::MidiMessage::noteOff (1, currentNote), 0);
                currentNote = -1;
            }
            countdown = 0.0;
            stepCounter = 0;
            lastStepNumber = -1;
            displayStep = -1;
            midi.swapWith (out);
            return;
        }

        // 3. Calage sur la grille de l'hote quand il joue
        if (hostPlaying)
        {
            const double phase = ppq / stepBeats;
            countdown = (std::ceil (phase - 1.0e-9) - phase) * stepSamples;
        }

        double pos = countdown;
        double offPos = currentNote >= 0 ? offCountdown : 1.0e18;

        while (pos < (double) numSamples)
        {
            long long stepNumber = stepCounter;
            if (hostPlaying)
                stepNumber = (long long) std::llround ((ppq + pos / samplesPerBeat) / stepBeats);

            if (hostPlaying && stepNumber == lastStepNumber)   // pas deja joue a la fin du bloc precedent
            {
                pos += stepSamples;
                continue;
            }

            const int offset = juce::jlimit (0, numSamples - 1, (int) pos);

            if (currentNote >= 0)
            {
                out.addEvent (juce::MidiMessage::noteOff (1, currentNote),
                              juce::jlimit (0, numSamples - 1, (int) juce::jmin (offPos, pos)));
                currentNote = -1;
            }

            buildSequence (s.pattern, s.octaves);
            if (! seq.empty())
            {
                const long long size = (long long) seq.size();
                const int idx = s.pattern == 3 ? rng.nextInt ((int) size)
                                               : (int) (((stepNumber % size) + size) % size);
                const int note = seq[(size_t) idx];
                out.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) lastVelocity), offset);
                currentNote = note;
                offPos = pos + gateSamples;
                displayStep = (int) (((stepNumber % 8) + 8) % 8);
            }

            lastStepNumber = stepNumber;
            pos += stepSamples;
            ++stepCounter;
        }

        if (currentNote >= 0 && offPos < (double) numSamples)
        {
            out.addEvent (juce::MidiMessage::noteOff (1, currentNote), juce::jlimit (0, numSamples - 1, (int) offPos));
            currentNote = -1;
        }

        countdown = pos - (double) numSamples;
        offCountdown = offPos - (double) numSamples;
        midi.swapWith (out);
    }

private:
    static double rateBeats (int rate)
    {
        static const double table[] = { 1.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125 };
        return table[juce::jlimit (0, 5, rate)];
    }

    void keyDown (int note, int vel, bool latch)
    {
        if (latch && down.empty())
            held.clear();                    // en mode Latch, un nouvel accord remplace l'ancien
        down.insert (note);
        lastVelocity = juce::jlimit (1, 127, vel);
        if (std::find (held.begin(), held.end(), note) == held.end())
            held.push_back (note);
    }

    void keyUp (int note, bool latch)
    {
        down.erase (note);
        if (! latch)
            held.erase (std::remove (held.begin(), held.end(), note), held.end());
    }

    void buildSequence (int pattern, int octaves)
    {
        seq.clear();
        auto sorted = held;
        std::sort (sorted.begin(), sorted.end());
        const int oct = juce::jlimit (1, 4, octaves);
        auto push = [this] (int n) { if (n >= 0 && n <= 127) seq.push_back (n); };

        switch (pattern)
        {
            case 1:   // Down
                for (int o = oct - 1; o >= 0; --o)
                    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
                        push (*it + 12 * o);
                break;

            case 2:   // Up-Down
            {
                std::vector<int> up;
                for (int o = 0; o < oct; ++o)
                    for (int n : sorted)
                        if (n + 12 * o <= 127) up.push_back (n + 12 * o);
                seq = up;
                for (int i = (int) up.size() - 2; i >= 1; --i)
                    seq.push_back (up[(size_t) i]);
                break;
            }

            case 4:   // As Played
                for (int o = 0; o < oct; ++o)
                    for (int n : held)
                        push (n + 12 * o);
                break;

            default:  // Up (et Random, qui pioche au hasard dans cette liste)
                for (int o = 0; o < oct; ++o)
                    for (int n : sorted)
                        push (n + 12 * o);
                break;
        }
    }

    std::vector<int> held, seq;
    std::set<int> down;
    int currentNote = -1, lastVelocity = 100, displayStep = -1;
    double countdown = 0.0, offCountdown = 0.0;
    long long stepCounter = 0, lastStepNumber = -1;
    juce::Random rng;
};
