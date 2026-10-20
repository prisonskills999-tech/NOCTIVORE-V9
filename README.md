# NOCTIVORE FX V9 — Cloud build for Windows 11 (Ableton Live)

**No Visual Studio, compiler, Git, JUCE, or Python installation required on your laptop.**

This repository contains the original v9 source. GitHub Actions downloads JUCE 8.0.15, synthesizes all 200 factory audio samples, compiles a Windows x64 VST3, and publishes two installable plugin artifacts.

## Build in your browser

1. Sign in to GitHub and create a **new private repository**. An empty repository is fine.
2. Click **Add file → Upload files**. Drag the **CONTENTS** of this extracted folder (including `Source` and `.github`), not the ZIP file and not the outer folder. Commit to your repository's default branch.
3. The workflow **Build NOCTIVORE V9 Windows VST3** runs automatically on push, or open the repository's **Actions** tab, select this workflow, then **Run workflow**.
4. Open the successful workflow run and download one artifact:
   - `NOCTIVORE-V9-FULL_GUI-WIN64-VST3` — custom earth-tone editor.
   - `NOCTIVORE-V9-SAFE_GUI-WIN64-VST3` — generic JUCE controls; use if your Ableton version crashes opening the custom editor.
5. Extract the downloaded artifact, then extract its contained `.zip`. Copy the resulting **entire** `NOCTIVORE FX SHADOW BIOME.vst3` folder to `C:\Program Files\Common Files\VST3`. Restart Ableton Live and rescan plugins.

**Install either FULL_GUI or SAFE_GUI, not both**: they use the same plugin identity. Close Ableton and remove an older v9 bundle first. Existing older NOCTIVORE versions with different identities can stay installed.

## Notes
- You will receive actual compiled Windows VST3 binaries from GitHub Actions **only after the hosted workflow successfully runs**.
- This is not an Ableton validation test. The FULL_GUI and SAFE_GUI builds are separate so the latter can be used if the custom GUI crashes.
- The sample generator reproduces the v9 original banks deterministically; neither G-Sonique audio nor proprietary samples are included.
- The GitHub run might consume Actions quota or require Actions to be enabled for your account/repository.
- If a CI job fails, the **first** CMake/MSVC compiler error in its log is the most useful diagnostic.
