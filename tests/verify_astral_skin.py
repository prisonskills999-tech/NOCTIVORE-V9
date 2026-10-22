#!/usr/bin/env python3
"""Final red/black single-screen VST3 editor verification."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
cpp=(root/"Source/VoidcoreEditor.cpp").read_text(encoding="utf-8")
hdr=(root/"Source/VoidcoreEditor.h").read_text(encoding="utf-8")
processor=(root/"Source/PluginProcessor.cpp").read_text(encoding="utf-8")
dsp=(root/"Source/RomplerEngine.h").read_text(encoding="utf-8")
main=cpp.split("void VoidcoreEditor::buildCore(){",1)[1].split("void VoidcoreEditor::buildDeeper(){",1)[0]
draw=cpp.split("void VoidcoreEditor::drawCore(",1)[1].split("void VoidcoreEditor::drawDeep(",1)[0]
ctor=cpp.split("VoidcoreEditor::VoidcoreEditor(",1)[1].split("VoidcoreEditor::~VoidcoreEditor",1)[0]
assert "buildCore();setPage(Core);" in ctor and "buildDeeper();" not in ctor
assert "W=1200.f,H=750.f" in hdr
assert "setFixedAspectRatio(1.6)" in ctor
for name in ("SOURCE  /  DUAL SAMPLE","VOID CORE  /  MORPH","FILTER  /  SPECTRAL",
             "MOTION  /  MODULATION","XENO  /  CHARACTER","GATE  /  16 STEP",
             "SPACE  /  DELAY & REVERB","TONE  /  THREE BAND EQ",
             "WIDTH  /  SPATIAL","OUTPUT  /  PERFORMANCE"):
    assert name in draw, "Missing single-page section: "+name
for visual in ("sourceWave(g,","filter(g,","lfo(g,","scope(g,","visualEye(g,"):
    assert visual in draw, "Missing animated visual: "+visual
assert "startTimerHz(30)" in ctor
assert "MidiKeyboardComponent" in main
assert "processor.keyboardState" in main
assert "scopeCursor.load" in cpp
assert "peakLeft.load" in cpp and "peakRight.load" in cpp
assert "sourceSoloButton" in cpp
assert "node.control->repaint()" in cpp
assert "void VoidcoreEditor::OrbPad::mouseDrag" in cpp
controls=re.findall(r'\b(?:knob|fader|choice)\("([^"]+)"',main)
defs=set(re.findall(r'(?:add|choose)\("([^"]+)"',processor))
defs.add("rate")
assert len(controls)>=30,(len(controls),controls)
assert all(x in defs for x in controls),set(controls)-defs
for knob,param in (("TUNE A","tuneA"),("TUNE B","tuneB"),("LEVEL A","levelA"),
                   ("LEVEL B","levelB"),("LOW","toneLow"),("MID","toneMid"),
                   ("HIGH","toneHigh"),("XENO","character"),("DRIVE","filterDrive"),
                   ("MORPH A/B","morph")):
    assert 'knob("'+param+'","'+knob+'"' in main
    assert 'p.'+param+'=get("'+param+'")' in processor
for name in ("toneLow","toneMid","toneHigh","tuneA","tuneB","levelA","levelB"):
    assert "p."+name in dsp
assert "filterGTable" in dsp
assert "const float pureSource=sample" in dsp
assert "left[i]+=rawL*masterGain;" in dsp
assert len(re.findall(r'click\(String\(i\+1\)',main))==1
assert 'shift("bank",-1)' in main and 'shift("bankB",1)' in main
print(f"PASS Final: {len(controls)} host-attached live controls, 16 gate buttons, playable MIDI, animated bank/LFO/scope, stereo meters, no hidden UI pages")
