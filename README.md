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

## Private Windows VST3 build without GitHub Actions

You can compile VOIDCORE without changing this repository to public, and without consuming GitHub-hosted Actions minutes.
Install Visual Studio 2022 with Desktop development with C++, CMake, and Python 3.10+ on your Windows x64 computer.
Use GitHub Desktop to clone the private repository and select branch feature/voidcore-astral-full-reskin, then from the project root run:

```powershell
powershell -ExecutionPolicy Bypass -File .\Build-VOIDCORE-Private.ps1
# Optional fallback; after creating the sound banks:
powershell -ExecutionPolicy Bypass -File .\Build-VOIDCORE-Private.ps1 -Edition SAFE_GUI -SkipBanks
```

The private builder runs the GUI preflight, generates and verifies 200 sound-bank WAVs, downloads public JUCE 8.0.15, compiles a Windows x64 VST3, verifies the bank files and native module, and creates a ZIP inside Private-Builds. It does not upload your private repository code. The script has not been tested on a Windows build machine here.

### Recover GitHub Actions for a private repository

Private GitHub repositories can build normally, but hosted runners use the account owner's included Actions minutes and billing budget. If jobs fail in seconds with runner_id 0 and no steps, check the account's Settings > Billing and licensing > Budgets and alerts for exhausted Actions usage, restrictive budgets or payment restrictions. Check Repository Settings > Actions > General for allowed workflow usage.

Both workflows on this branch require manually choosing Run workflow to avoid automatically consuming private-runner minutes on each commit. If billing permits, use Actions and choose this branch to request a build. Local or self-hosted builds do not use GitHub-hosted runner minutes.

## Build Windows VST3

Open [GitHub Actions](../../actions/workflows/windows-vst3.yml), select this branch and choose **Run workflow** for **Build VOIDCORE Astral Xeno VST3**. Both FULL_GUI and SAFE_GUI variants are produced. Automatic push builds are disabled to conserve private Actions minutes.

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
