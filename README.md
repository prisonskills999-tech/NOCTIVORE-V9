# VOIDCORE V12 — XENO Soundscapes

**Windows x64 VST3 synthesizer** built with JUCE 8.0.15. Includes the interactive Matrix GUI (neon green, black, white) with live sound-sphere animation, working bank controls, MIDI keyboard, animated modulation, and stereo meters.

## What changed in V12

The earlier generator blended a common tonal base into many banks, which made the instrument sound too uniform. The new **`Source/generate_xeno_soundscapes.py`** uses ten independent synthesis families, with five mutations per family:

| Family | Sound |
|---|---|
| Abyssal Gravity | Massive slow sub-void drones |
| Xeno Liquids | Pitch-diving wet alien bubbles |
| Alien Transmissions | Coded signals and fragmented radio textures |
| Creature Choirs | Morphing vocal-like alien throats |
| Crystal Entities | Inharmonic glass and resonant shards |
| Insect Hive | Fast alien chirps and microorganism swarms |
| Pulsar Rituals | Deep gated ceremonial pulses |
| Wormhole Motion | Spectral warps and frequency sweeps |
| Nebula Choirs | Evolving smooth space pads |
| Dimensional Glitch | Broken digital fragments and stutters |

The 50 original bank **indices** and four note roots (36, 48, 60, 72) are preserved to keep preset bank selection compatible. Sounds and display names are deliberately different. All 200 WAVs are original deterministic synthesis (48 kHz PCM16), not third-party samples.

The System page's four scene buttons now select contrasting A/B bank combinations and distinct FX treatments. **MUTATE** also chooses new bank families; **EVOLVE** makes gentler modifications.

## Build Windows VST3

Open [GitHub Actions](../../actions/workflows/windows-vst3.yml) and run **Build VOIDCORE v12 Xeno Soundscapes VST3** from this branch, or push a commit. Both FULL_GUI and SAFE_GUI build variants are produced.

The Windows workflow:
1. Runs source-level GUI checks.
2. Installs Python/Numpy/Scipy.
3. Creates 200 new XENO WAVs and runs `tests/verify_xeno_diversity.py`.
4. Compiles JUCE 8.0.15 x64 Windows VST3.
5. Packages `Contents/Resources/Banks/*.wav` into the final installable VST3.
6. Uploads `VOIDCORE-V12-XENO-FULL_GUI-WIN64-VST3` and the safe fallback artifact.

For a local sound-only generation:
```bash
python -m pip install -r requirements.txt
python Source/generate_xeno_soundscapes.py
python tests/verify_xeno_diversity.py
```

## Installation and verification

Download FULL_GUI under the successful GitHub Actions run's **Artifacts** section, unzip it, and copy the extracted `.vst3` bundle into `C:\Program Files\Common Files\VST3`. Rescan your plugin host. Keep the `Banks` folder **inside** the VST3 bundle, exactly as packaged.

Test A/B selections such as **00+05**, **03+09**, **01+08**, and **02+06** to hear contrasting families. Use the System scene buttons for four preset combinations.

GitHub Actions build failures with zero steps are usually runner/account setup failures, not compiler diagnostics. For private repositories, check Actions permissions, billing and runner usage. The repository does not need to be public for a GitHub-connected assistant to edit it.

This source branch is for review and testing. Actual DAW playback and animation require host-side QA; compilation alone cannot verify sound quality or UI behavior.
