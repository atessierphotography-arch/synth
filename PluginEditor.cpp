#include "PluginEditor.h"

namespace
{
    const juce::Colour textHi  (0xffe8e8ec);
    const juce::Colour textLo  (0xff9a9aa4);
    const juce::Colour textDim (0xff8a8a94);

    constexpr int W = 1100, H = 580;

    // Formats d'affichage des valeurs
    enum Fmt { Pct, Semi, Cents, Int, Freq, Time, Db, Hz2, Oct, SignedPct };

    struct KnobDef { const char* id; const char* label; int cx, cy, size; Fmt fmt; };

    const KnobDef knobDefs[] =
    {
        // Oscillateurs
        { "semiB",     "SEMI B",    74, 190, 58, Semi },     { "detune",   "DETUNE",   150, 190, 58, Cents },
        { "unison",    "UNISON",   226, 190, 58, Int },      { "width",    "WIDTH",    302, 190, 58, Pct },
        { "mixB",      "OSC MIX",  378, 190, 58, Pct },
        // Filtre
        { "cutoff",    "CUTOFF",   478, 190, 58, Freq },     { "res",      "RESO",     554, 190, 58, Pct },
        { "envAmt",    "ENV AMT",  630, 190, 58, SignedPct }, { "fltDecay", "FLT DEC",  706, 190, 58, Time },
        // Enveloppe
        { "attack",    "ATTACK",   806, 190, 58, Time },     { "decay",    "DECAY",    882, 190, 58, Time },
        { "sustain",   "SUSTAIN",  958, 190, 58, Pct },      { "release",  "RELEASE", 1034, 190, 58, Time },
        // LFO
        { "lfoRate",   "RATE",      74, 395, 58, Hz2 },      { "lfoFilter", "TO FILTER", 150, 395, 58, Pct },
        { "lfoPitch",  "TO PITCH", 226, 395, 58, Pct },
        // Effets
        { "chorus",    "CHORUS",   328, 395, 58, Pct },      { "delayTime", "DLY TIME", 404, 395, 58, Time },
        { "delayMix",  "DLY MIX",  480, 395, 58, Pct },      { "revSize",  "REV SIZE", 556, 395, 58, Pct },
        { "revMix",    "REV MIX",  632, 395, 58, Pct },
        // Arpegiateur
        { "arpOct",    "OCTAVES",  744, 395, 58, Oct },      { "arpGate",  "GATE",     828, 395, 58, Pct },
        // General
        { "master",    "MASTER",  1040,  36, 52, Db }
    };

    juce::Font bold (float h, float kern = 0.1f)
    {
        return juce::Font (juce::FontOptions().withHeight (h).withStyle ("Bold")).withExtraKerningFactor (kern);
    }
    juce::Font regular (float h, float kern = 0.05f)
    {
        return juce::Font (juce::FontOptions().withHeight (h)).withExtraKerningFactor (kern);
    }

    juce::String formatValue (Fmt fmt, double v)
    {
        const juce::String plus = v > 0.0 ? "+" : "";
        switch (fmt)
        {
            case Pct:       return juce::String (juce::roundToInt (v)) + " %";
            case Semi:      return plus + juce::String (juce::roundToInt (v)) + " st";
            case Cents:     return juce::String (juce::roundToInt (v)) + " ct";
            case Int:       return juce::String (juce::roundToInt (v));
            case Freq:      return v < 1000.0 ? juce::String (juce::roundToInt (v)) + " Hz" : juce::String (v / 1000.0, 1) + " kHz";
            case Time:      return v < 1000.0 ? juce::String (juce::roundToInt (v)) + " ms" : juce::String (v / 1000.0, 2) + " s";
            case Db:        return plus + juce::String (v, 1) + " dB";
            case Hz2:       return juce::String (v, 2) + " Hz";
            case Oct:       return juce::String (juce::roundToInt (v)) + " oct";
            case SignedPct: return plus + juce::String (juce::roundToInt (v)) + " %";
        }
        return {};
    }

    void drawDivider (juce::Graphics& g, int x, int y, int w, int h)
    {
        g.setColour (juce::Colour (0xff0b0b0d));
        g.fillRect (x, y, w, h);
        g.setColour (juce::Colour (0xff3c3c42));
        g.fillRect (x + (w == 1 ? 1 : 0), y + (h == 1 ? 1 : 0), w, h);
    }

    void drawTitle (juce::Graphics& g, const char* text, int x, int y)
    {
        g.setColour (textDim);
        g.setFont (bold (12.0f, 0.3f));
        g.drawText (text, x, y, 300, 16, juce::Justification::centredLeft);
    }

    void drawCaption (juce::Graphics& g, const char* text, int x, int y)
    {
        g.setColour (textDim);
        g.setFont (bold (10.0f, 0.2f));
        g.drawText (text, x, y, 120, 14, juce::Justification::centredLeft);
    }

    const juce::Rectangle<int> stepArea (900, 378, 156, 22);
}

//==============================================================================
WarlordSynthLookAndFeel::WarlordSynthLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff0e0e10));
    setColour (juce::ComboBox::textColourId, textHi);
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff3a3a40));
    setColour (juce::ComboBox::arrowColourId, juce::Colour (0xffffb347));
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1b1b1e));
    setColour (juce::PopupMenu::textColourId, textHi);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff45454b));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void WarlordSynthLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                                float startA, float endA, juce::Slider&)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float k = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 100.0f;
    const auto c = bounds.getCentre();
    const float angle = startA + pos * (endA - startA);

    auto pt = [&] (float r, float a) { return juce::Point<float> (c.x + r * k * std::sin (a), c.y - r * k * std::cos (a)); };

    // Reperes : 11 traits, allumes jusqu'a la valeur
    for (int t = 0; t < 11; ++t)
    {
        const float a = startA + (endA - startA) * (float) t / 10.0f;
        g.setColour ((float) t / 10.0f <= pos + 0.001f ? accent : juce::Colour (0xff5a5a62));
        g.drawLine (juce::Line<float> (pt (43.0f, a), pt (48.0f, a)), 2.0f * k);
    }

    // Corps metal
    juce::ColourGradient body (juce::Colour (0xff5c5c63), c.x, c.y - 15.0f * k, juce::Colour (0xff1a1a1d), c.x, c.y + 40.0f * k, true);
    g.setGradientFill (body);
    g.fillEllipse (c.x - 36.0f * k, c.y - 36.0f * k, 72.0f * k, 72.0f * k);
    g.setColour (juce::Colour (0xff09090a));
    g.drawEllipse (c.x - 36.0f * k, c.y - 36.0f * k, 72.0f * k, 72.0f * k, 2.0f);

    // Stries de prise
    g.setColour (juce::Colour (0xff0e0e10).withAlpha (0.55f));
    for (int i = 0; i < 72; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / 72.0f;
        g.drawLine (juce::Line<float> (pt (32.5f, a), pt (35.5f, a)), 1.2f * k);
    }

    // Face superieure
    juce::ColourGradient face (juce::Colour (0xff46464c), c.x, c.y - 27.0f * k, juce::Colour (0xff232326), c.x, c.y + 27.0f * k, false);
    g.setGradientFill (face);
    g.fillEllipse (c.x - 27.0f * k, c.y - 27.0f * k, 54.0f * k, 54.0f * k);
    g.setColour (juce::Colour (0xff0e0e10));
    g.drawEllipse (c.x - 27.0f * k, c.y - 27.0f * k, 54.0f * k, 54.0f * k, 1.5f);

    // Trait repere
    juce::Path mark;
    mark.startNewSubPath (pt (14.0f, angle));
    mark.lineTo (pt (26.0f, angle));
    g.setColour (juce::Colour (0xfff2f2f4));
    g.strokePath (mark, juce::PathStrokeType (3.5f * k, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void WarlordSynthLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    const auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool isToggle = (bool) b.getProperties().getWithDefault ("led", false);
    const bool on = isToggle && b.getToggleState();

    if (on)
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff45454b), 0.0f, r.getY(), juce::Colour (0xff2c2c30), 0.0f, r.getBottom(), false));
    else
        g.setColour (down ? juce::Colour (0xff2c2c30) : (over ? juce::Colour (0xff26262a) : juce::Colour (0xff1c1c1f)));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (on ? accent.withAlpha (0.8f) : juce::Colour (0xff3a3a40));
    g.drawRoundedRectangle (r, 6.0f, 1.5f);

    if (isToggle)   // voyant LED
    {
        const juce::Point<float> led (r.getX() + 15.0f, r.getCentreY());
        if (on)
        {
            g.setColour (accent.withAlpha (0.3f));
            g.fillEllipse (led.x - 7.0f, led.y - 7.0f, 14.0f, 14.0f);
        }
        g.setColour (on ? accent : juce::Colour (0xff2c2c30));
        g.fillEllipse (led.x - 4.0f, led.y - 4.0f, 8.0f, 8.0f);
    }
}

void WarlordSynthLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    const bool isToggle = (bool) b.getProperties().getWithDefault ("led", false);
    g.setFont (getTextButtonFont (b, b.getHeight()));
    g.setColour (isToggle && ! b.getToggleState() ? textDim : juce::Colours::white);
    g.drawText (b.getButtonText(), b.getLocalBounds().withTrimmedLeft (isToggle ? 18 : 0), juce::Justification::centred);
}

juce::Font WarlordSynthLookAndFeel::getTextButtonFont (juce::TextButton&, int) { return bold (14.0f, 0.15f); }
juce::Font WarlordSynthLookAndFeel::getComboBoxFont (juce::ComboBox&)          { return bold (13.0f, 0.05f); }
juce::Font WarlordSynthLookAndFeel::getPopupMenuFont()                          { return regular (15.0f); }

//==============================================================================
WarlordSynthEditor::WarlordSynthEditor (WarlordSynthProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Boutons rotatifs
    for (const auto& def : knobDefs)
    {
        auto s = std::make_unique<juce::Slider>();
        s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s->setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
        s->setLookAndFeel (&laf);
        s->onValueChange = [this] { repaint (30, 40, W - 60, 460); };   // met a jour les valeurs affichees
        addAndMakeVisible (*s);
        knobAttach.push_back (std::make_unique<SliderAttach> (proc.apvts, def.id, *s));

        if (auto* param = proc.apvts.getParameter (def.id))   // double-clic = valeur par defaut
            s->setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));
        knobs.push_back (std::move (s));
    }

    // Listes deroulantes : Wave A, Wave B, Pattern, Rate
    const char* comboIds[] = { ids::waveA, ids::waveB, ids::arpPattern, ids::arpRate };
    for (size_t i = 0; i < combos.size(); ++i)
    {
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (comboIds[i])))
            combos[i].addItemList (choice->choices, 1);
        combos[i].setLookAndFeel (&laf);
        addAndMakeVisible (combos[i]);
        comboAttach.push_back (std::make_unique<ComboAttach> (proc.apvts, comboIds[i], combos[i]));
    }

    // Boutons ARP / LATCH (avec voyant)
    for (auto* b : { &arpButton, &latchButton })
    {
        b->setClickingTogglesState (true);
        b->getProperties().set ("led", true);
        b->setLookAndFeel (&laf);
        addAndMakeVisible (*b);
    }
    buttonAttach.push_back (std::make_unique<ButtonAttach> (proc.apvts, ids::arpOn, arpButton));
    buttonAttach.push_back (std::make_unique<ButtonAttach> (proc.apvts, ids::arpLatch, latchButton));

    // Presets
    for (auto* b : { &prevButton, &nextButton })
    {
        b->setLookAndFeel (&laf);
        addAndMakeVisible (*b);
    }
    prevButton.onClick = [this] { changePreset (-1); };
    nextButton.onClick = [this] { changePreset (+1); };

    // Clavier a l'ecran
    keyboard.setKeyWidth (28.0f);
    keyboard.setAvailableRange (36, 96);
    keyboard.setLowestVisibleKey (36);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, laf.accent.withAlpha (0.7f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, laf.accent.withAlpha (0.25f));
    addAndMakeVisible (keyboard);

    setSize (W, H);
    startTimerHz (30);
}

WarlordSynthEditor::~WarlordSynthEditor()
{
    for (auto& s : knobs) s->setLookAndFeel (nullptr);
    for (auto& c : combos) c.setLookAndFeel (nullptr);
    arpButton.setLookAndFeel (nullptr);
    latchButton.setLookAndFeel (nullptr);
    prevButton.setLookAndFeel (nullptr);
    nextButton.setLookAndFeel (nullptr);
}

void WarlordSynthEditor::changePreset (int delta)
{
    const int n = proc.getNumPrograms();
    proc.setCurrentProgram ((proc.getCurrentProgram() + delta + n) % n);
    repaint();
}

void WarlordSynthEditor::resized()
{
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const auto& d = knobDefs[i];
        knobs[i]->setBounds (d.cx - d.size / 2, d.cy - d.size / 2, d.size, d.size);
    }

    combos[0].setBounds (40, 118, 110, 26);     // Wave A
    combos[1].setBounds (160, 118, 110, 26);    // Wave B
    combos[2].setBounds (870, 312, 96, 28);     // Arp pattern
    combos[3].setBounds (974, 312, 84, 28);     // Arp rate
    arpButton.setBounds (708, 312, 70, 28);
    latchButton.setBounds (786, 312, 76, 28);

    prevButton.setBounds (396, 17, 36, 36);
    nextButton.setBounds (688, 17, 36, 36);
    keyboard.setBounds (34, 488, 1032, 76);
}

void WarlordSynthEditor::timerCallback()
{
    const int step = proc.arpStep.load();
    if (step != lastStep)
    {
        lastStep = step;
        repaint (stepArea);
    }
}

void WarlordSynthEditor::paint (juce::Graphics& g)
{
    const auto accent = laf.accent;

    // Panneau metal
    g.fillAll (juce::Colour (0xff0d0d0e));
    const auto panel = getLocalBounds().toFloat().reduced (1.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff323236), 0.0f, 0.0f, juce::Colour (0xff1b1b1e), 0.0f, (float) H, false));
    g.fillRoundedRectangle (panel, 10.0f);
    {
        juce::Path clipPath;
        clipPath.addRoundedRectangle (panel, 10.0f);
        g.saveState();
        g.reduceClipRegion (clipPath);
        const auto clip = g.getClipBounds();
        g.setColour (juce::Colours::white.withAlpha (0.025f));
        for (int x = clip.getX() - (clip.getX() % 3); x < clip.getRight(); x += 3)
            g.fillRect (x, clip.getY(), 1, clip.getHeight());
        g.restoreState();
    }
    g.setColour (juce::Colour (0xff09090a));
    g.drawRoundedRectangle (panel, 10.0f, 2.0f);
    g.setColour (juce::Colour (0xff4c4c52));
    g.drawLine (10.0f, 3.0f, (float) W - 10.0f, 3.0f);

    // Vis
    for (auto p : { juce::Point<float> (12, 12), { 1078, 12 }, { 12, 558 }, { 1078, 558 } })
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff77777e), p.x + 3.5f, p.y + 3.0f, juce::Colour (0xff232326), p.x + 10.0f, p.y + 10.0f, true));
        g.fillEllipse (p.x, p.y, 10.0f, 10.0f);
        g.setColour (juce::Colour (0xff0b0b0c));
        g.drawEllipse (p.x, p.y, 10.0f, 10.0f, 1.0f);
    }

    // Titre
    g.setColour (juce::Colour (0xfff2f2f4));
    g.setFont (bold (30.0f, 0.15f));
    g.drawText ("WARLORD SYNTH", 34, 10, 340, 36, juce::Justification::centredLeft);
    g.setColour (textDim);
    g.setFont (bold (11.0f, 0.3f));
    g.drawText ("SYNTH + ARPEGGIATOR + PADS", 36, 46, 340, 16, juce::Justification::centredLeft);

    // Preset courant
    const juce::Rectangle<int> presetBox (440, 17, 240, 36);
    g.setColour (juce::Colour (0xff0e0e10));
    g.fillRoundedRectangle (presetBox.toFloat(), 6.0f);
    g.setColour (juce::Colour (0xff3a3a40));
    g.drawRoundedRectangle (presetBox.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setColour (accent);
    g.setFont (bold (15.0f, 0.12f));
    g.drawText (proc.getProgramName (proc.getCurrentProgram()).toUpperCase(), presetBox, juce::Justification::centred);

    // Separateurs et titres de sections
    drawDivider (g, 34, 78, W - 68, 1);
    drawDivider (g, 34, 266, W - 68, 1);
    drawDivider (g, 428, 90, 1, 166);
    drawDivider (g, 756, 90, 1, 166);
    drawDivider (g, 277, 280, 1, 190);
    drawDivider (g, 684, 280, 1, 190);

    drawTitle (g, "OSCILLATORS", 34, 86);
    drawTitle (g, "FILTER", 440, 86);
    drawTitle (g, "AMP ENVELOPE", 770, 86);
    drawTitle (g, "LFO", 34, 276);
    drawTitle (g, "EFFECTS", 290, 276);
    drawTitle (g, "ARPEGGIATOR", 700, 276);

    drawCaption (g, "WAVE A", 40, 102);
    drawCaption (g, "WAVE B", 160, 102);
    drawCaption (g, "PATTERN", 870, 298);
    drawCaption (g, "RATE", 974, 298);

    // Noms et valeurs des boutons
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const auto& d = knobDefs[i];
        const juce::String value = formatValue (d.fmt, knobs[i]->getValue());

        if (juce::String (d.id) == "master")   // volume general, a droite de l'en-tete
        {
            g.setColour (textHi);
            g.setFont (bold (12.0f, 0.15f));
            g.drawText (d.label, 920, 16, 76, 16, juce::Justification::centredRight);
            g.setColour (accent);
            g.setFont (regular (13.0f));
            g.drawText (value, 920, 34, 76, 16, juce::Justification::centredRight);
            continue;
        }

        const int top = d.cy + d.size / 2 + 6;
        g.setColour (textHi);
        g.setFont (bold (12.0f, 0.1f));
        g.drawText (d.label, d.cx - 40, top, 80, 16, juce::Justification::centred);
        g.setColour (textLo);
        g.setFont (regular (12.0f));
        g.drawText (value, d.cx - 40, top + 16, 80, 16, juce::Justification::centred);
    }

    // Voyants de pas de l'arpegiateur
    drawCaption (g, "STEP", 900, 404);
    const int step = proc.arpStep.load();
    for (int j = 0; j < 8; ++j)
    {
        const juce::Rectangle<float> led ((float) (stepArea.getX() + j * 19), (float) stepArea.getY() + 3.0f, 12.0f, 16.0f);
        if (j == step)
        {
            g.setColour (accent.withAlpha (0.3f));
            g.fillRoundedRectangle (led.expanded (2.0f), 3.0f);
        }
        g.setColour (j == step ? accent : juce::Colour (0xff26262a));
        g.fillRoundedRectangle (led, 2.0f);
    }
}
