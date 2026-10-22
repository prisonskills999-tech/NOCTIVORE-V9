# VOIDCORE v11 — Live JUCE VST3 editor

## A real plugin, not the generated picture

This branch replaces the obsolete illustrated NOCTIVORE editor with a procedural JUCE editor, compiled in \`Source/VoidcoreEditor.cpp\`. The old screenshot is **not** drawn and is **not** shipped. All hit areas and the visible UI use the same 1200×800 logical coordinate system, a constrained 3:2 aspect ratio, and a default 1050×700 window.

### CORE (ten knobs, two selectors, one morph fader)

Source A and B choose any of 50 original multisample banks plus User. Both waveform panes load the actual 60-MIDI-note WAV of the selected bank from \`Contents/Resources/Banks\`. A moving play cursor overlays each waveform; if the file is missing, "NO BANK AUDIO FOUND" is displayed rather than faking a waveform. Source Start, Alien Throat, Texture and Source Coupling have real parameter attachments. The morph fader and six performance knobs (Filter, Resonance, Creatures, Evolve, Space and Output) send full host automation.

A biomechanical eye pulses with the actual peak meter. The bottom scope uses real audio samples from the processor's atomic display ring. The stereo meter reacts to the audio output.

### MOTION

The four LFO tabs switch between independently attached rate/depth/shape/sync/target controls. The LFO waveform is animated from its parameters. ADSR amplitude stages are displayed as a curve and edited by the four actual envelope knobs. The 16 rhythm steps are buttons connected to host-saved \`gateStep01..16\` values; the graphical step playhead reads host PPQ. Rhythm pattern, note division, gate strength and swing are editable.

### FX

Delay (mix, feedback, tone, host tempo division), reverb (mix, size, decay), texture movement and four alien creature macros. Delay/reverb graphical traces animate with their feedback settings. Legacy and expert parameters remain accessible through DAW automation or the generic fallback editor, rather than overwhelming the MAIN page.

### SYSTEM

Voice limits (8/12/16/24), poly/mono/legato, quality, reverse, glide, pan, width, clarity, four creature scenes, Save and Load patch through the native file chooser. Save/Load stores or restores the processor's full VST3 state as a \`.nfx\` file. Global Save, Load, Mutate and Evolve buttons are available from all pages.

## Audio

Carries forward the original 200-sample organic/alien sound engine and the Smooth-512 improvements, while removing random grain-anchor jumps and hard spectral wavefolding completely. Playback always uses cubic sample interpolation for quality. No user-supplied or third-party audio is copied.

## Building

\`.github/workflows/windows-vst3.yml\` attempts FULL_GUI and SAFE_GUI on Windows. The desired output is **VOIDCORE-V11-FULL_GUI-WIN64-VST3**, containing the actual VST3 folder and the 200 WAV bank files. The project uses JUCE 8.0.15.

**As of October 22, 2026: GitHub Actions runs for this branch fail before being assigned a runner (runner_id=0, no steps, even the independent Linux compile check).** These runs do not establish either compilation success or failure of the C++ GUI. The source is committed, but a verified Windows binary is not available from those runs. Inspect repository Actions settings, usage limits and runner availability in GitHub before treating the ZIP as ready.

## Manual visual/functional QA once cloud runners are working

1. Open FULL_GUI in Ableton Live, resize it between 900×600 and 1200×800 and check knob/hitbox alignment.
2. Select Bank A and B and verify each sample waveform changes; press and sustain C3 and check moving scope and level meter.
3. Select each of LFO1–4, automate waveform/rate/depth, verify a different curve and actual modulation.
4. Change ADSR and hear its stage envelopes; click each of the 16 gate buttons and verify host recall after reopening a project.
5. Drag morph, change filters and effects; verify DAW parameter automation and no audio-thread allocation.
6. Save an NFX patch, mutate controls, load the NFX patch and confirm values restored.
7. Compare offline and realtime exports at buffer sizes 128, 256 and 512 with CPU headroom.
