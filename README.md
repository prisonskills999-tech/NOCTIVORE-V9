# NOCTIVORE FX v10.3 — FOREST SIGNALS
Original darkpsy/forest-psy instrument. Maintains the v10.2 hand-painted JUCE GUI,
the stable v10.1 low-register modulation, four LFOs, dual filters, and MIDI clock gate.

## New in this version
- 50 entirely re-authored banks, 25 genuinely different organic synthesis topologies,
  each with two independent performances; 200 48 kHz factory WAV multisamples.
- Evolving wet forest bleeps, mellow blops, fungal voices, singing insects,
  root transmissions, hollow reeds and moving psychedelic spectral textures.
- Longer recorded sample swells (~170ms), much gentler MIDI note onset,
  longer default attack and release, higher sustain.
- Metal resonator and gate-triggered pitched events OFF by default to suppress
  the recurring 'boing'; FM defaults subdued but remains available.
- Rhythmic gate/sync grid, Deep Life, Linked Motion, and the full effects retained.
- All banks are synthesized from original code. No G-Sonique library or recordings.

## Windows build
Use .github/workflows/windows-vst3.yml in this repository branch;
GitHub Actions generates all 200 WAVs, builds the JUCE VST3 on Windows,
and uploads FULL_GUI and SAFE_GUI VST3 artifacts.
Build must complete successfully and be tested in Ableton to be considered verified.
