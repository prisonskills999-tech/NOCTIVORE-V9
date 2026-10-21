# NOCTIVORE FX v10 — XENOBIOME

**Original darkpsy/forest-psy soundscape instrument for Windows 11 VST3 (JUCE 8).**

Rebuilt from the working v9 audio/GUI architecture. This edition addresses the persistent moving hiss on every patch: the sample generator produces zero stochastic broadband-noise arrays, the rhythmic ghost noise burst was replaced by a tuned resonant event, and the continuous synthesized per-voice breath-noise layer was removed. The gate and sync grid remain.

## What sounds different

- **25 fundamentally different organic models / 50 sound banks**, 4 pitch-zone 48 kHz WAVs per bank (200 original samples); more differences in envelope, voicing and spectral shape.
- **Noise-free samples**: resonant oscillators, cross-modulated formants, non-harmonic sine clusters, glottal membranes, liquid FM, insect trills and tribal pulses, rather than continuous filtered white/pink noise.
- **Alien Vocal Tract**: morphable dual resonant peaks that shift slowly with Living Biome.
- **Organism Coupling**: harmonic cross-excitation between bank A and bank B, rather than just crossfading.
- **Body Mutation**: varies interacting organic motion, including subtle FM variation.
- 4 ultra-slow LFOs, dual filters, A/B morph, rhythmic gate, host sync, psychedelic delay/reverb retained.
- FX noise bursts from the old "Ghost Texture" button are now pitched/resonant events. Existing automation parameter ID `motifFX` remains unchanged, but the label reads `Resonant Events`.

## Build without installing developer software

Upload all repository source files to GitHub (including `.github/workflows/windows-vst3.yml`). The Actions workflow generates the samples on a hosted Windows computer, compiles JUCE VST3, and uploads **FULL_GUI** and **SAFE_GUI** Windows x64 plugin bundles as artifacts. No local Visual Studio is needed. The Windows binary must successfully compile in GitHub Actions before it can be considered tested.

Build commands for developers:

```sh
python -m pip install -r requirements.txt
python Source/generate_shadow_biome.py
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target NOCTIVORE_FX_VST3
```

Original sounds and architecture; no G-Sonique code or recordings. This is inspired by the exploratory darkpsy FX instrument concept, not a sample-exact FX466 clone.
