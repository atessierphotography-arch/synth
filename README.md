# Warlord Synth

Synthe (VST3 + Standalone) avec arpegiateur et pads, construit avec JUCE 8. 16 voix, sortie stereo.

## Son
- 2 oscillateurs (Saw, Square, Triangle, Sine), decalage en demi-tons du 2e, detune
- Unisson de 1 a 5 voix avec ecartement stereo (Width)
- Filtre passe-bas avec resonance, enveloppe de filtre (Env Amt, Flt Dec), LFO vers filtre et hauteur
- Enveloppe d'amplitude ADSR (attaques jusqu'a 8 s, releases jusqu'a 10 s pour les pads)
- Effets : chorus, delay, reverb ; volume general (Master)

## Arpegiateur
ARP on/off, LATCH (garde les notes apres relachement), Pattern (Up, Down, Up-Down, Random, As Played),
Rate (1/4, 1/8, 1/8T, 1/16, 1/16T, 1/32), Octaves (1 a 4), Gate.
Cale sur le tempo de l'hote : quand la lecture tourne, les pas suivent la grille du projet.

## Presets (fleches < > en haut)
Warm Pad, Dark Pad, Glass Pad, String Ensemble, Airy Pad, Evolving Pad, Sub Pad, Super Saw Pad,
Ambient Wash, Retro Pad, Arp Pluck, Arp Pulse, Lead Synth, Bass Synth.

Un clavier a l'ecran (bas de la fenetre) permet d'essayer les sons sans clavier MIDI.

## Compiler (Windows, Visual Studio 2022 + CMake)
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Le VST3 est copie dans `C:\Program Files\Common Files\VST3` (compilation en administrateur,
ou ajoute `-DCOPY_AFTER_BUILD=OFF` et copie le dossier a la main).
Dans Reaper : Options > Preferences > Plug-ins > VST > Re-scan, puis cree une piste et ajoute
« Warlord Synth » avec FX > Add (ou Insert > Virtual instrument on new track).

Sans installer Visual Studio : mets ces fichiers sur GitHub et utilise le workflow `build.yml`
(onglet Actions > Artifacts).
