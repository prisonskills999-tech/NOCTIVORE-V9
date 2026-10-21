# NOCTIVORE v10.4 — TRUE PAINTED FOREST

**What changed:** The main Atlas GUI renders the original hand-painted artwork from `Assets/painted_forest.jpg`, instead of drawing a rough vector imitation. Forty-nine? More than forty visible knobs are host-automatable interactive sliders aligned to the illustrated knobs; banks, gate patterns, effect selectors and five tabs are interactive. All deep modulation, effects, filter and utility pages are preserved.

**IMPORTANT: ONE ARTWORK UPLOAD REQUIRED.** To complete this branch without installing a compiler, add the supplied JPEG to `Assets/painted_forest.jpg` using GitHub → Add file → Upload files (select this branch). Actions builds automatically. Until that picture is added, CMake deliberately fails so there is no misleading GUI artifact.

**Choose the FULL_GUI artifact.** SAFE_GUI intentionally uses the generic parameter editor and will never show the artwork. After the build, install the full plugin with its `Contents/Resources/GUI/painted_forest.jpg`.

The painting is raster and static; the controls over it are live and mapped to real APVTS parameters. Some illustrated details remain decorative, and there are no fake audio modulations.

Windows compilation and Ableton host loading must still be verified.
