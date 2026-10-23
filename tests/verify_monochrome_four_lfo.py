#!/usr/bin/env python3
"""Monochrome one-page FOUR LFO + meaningful room/delay controls."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
editor=(root/"Source/VoidcoreEditor.cpp").read_text()
hdr=(root/"Source/VoidcoreEditor.h").read_text()
params=(root/"Source/PluginProcessor.cpp").read_text()
space=(root/"Source/PsySpace.h").read_text()
engine=(root/"Source/RomplerEngine.h").read_text()
main=editor.split("void VoidcoreEditor::buildCore(){",1)[1].split("void VoidcoreEditor::buildDeeper(){",1)[0]
assert "buildCore();setPage(Core);" in editor and "buildCore();buildDeeper();" not in editor
assert "W=1200.f,H=900.f" in hdr
assert "for(int i=0;i<4;++i)" in main and 'click("LFO "+String(i+1)' in main
assert "20+focusLfo" in editor and "node.onPage=20+i" not in editor
assert main.count("ui.back().onPage=20+i")>=5
assert "lfo(g,{43,443,300,28},focusLfo)" in editor
assert "clampf(cfg.rate,.00001f,3.f)" in engine
assert "NormalisableRange<float>(.00001f,3.f" in params
for param in ("delayImpact","delayDrift","delayMode","reverbImpact","reverbMode","reverbWidth"):
 assert 'add("'+param+'"' in params and 'p.fx.'+param+'=get("'+param+'")' in params
 assert ('choice("'+param+'"' in main or 'knob("'+param+'"' in main)
 assert 'c.'+param in space
for name in ("sourceWave(g,","scope(g,","lfo(g,","filter(g,"):
 assert name in editor
# Do not permit colored RGB tuples in paint implementation.
for red,green,blue in re.findall(r"Colour::fromRGB\((\d+),(\d+),(\d+)\)",editor+hdr):
 assert red==green==blue, f"Nonmonochrome color: {red},{green},{blue}"
assert 'MidiKeyboardComponent' in main
assert 'panel(g,{16,593,558,145}' in editor
print("PASS: four selectable LFOs, deep rates, six FX macros, live visuals, grayscale-only one-page interface")
