#pragma once
#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <vector>

// Look "materiel de studio" : panneau metal sombre, gros boutons a stries, voyants LED
class WarlordSynthLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WarlordSynthLookAndFeel();

    juce::Colour accent { 0xffffa726 };

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
};

class WarlordSynthEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit WarlordSynthEditor (WarlordSynthProcessor&);
    ~WarlordSynthEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttach  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    void changePreset (int delta);

    WarlordSynthProcessor& proc;
    WarlordSynthLookAndFeel laf;

    std::vector<std::unique_ptr<juce::Slider>> knobs;
    std::vector<std::unique_ptr<SliderAttach>> knobAttach;

    std::array<juce::ComboBox, 4> combos;   // 0 Wave A, 1 Wave B, 2 Arp pattern, 3 Arp rate
    std::vector<std::unique_ptr<ComboAttach>> comboAttach;

    juce::TextButton arpButton { "ARP" }, latchButton { "LATCH" }, prevButton { "<" }, nextButton { ">" };
    std::vector<std::unique_ptr<ButtonAttach>> buttonAttach;

    juce::MidiKeyboardComponent keyboard;
    int lastStep = -2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WarlordSynthEditor)
};
