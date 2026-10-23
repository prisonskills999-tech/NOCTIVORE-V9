#!/usr/bin/env python3
"""Guarantee the shipped GUI has weighted, legible, real precision controls."""
from pathlib import Path
import re
base=Path(__file__).resolve().parents[1]
e=(base/"Source/VoidcoreEditor.cpp").read_text(encoding="utf-8")
h=(base/"Source/VoidcoreEditor.h").read_text(encoding="utf-8")
p=(base/"Source/PluginProcessor.cpp").read_text(encoding="utf-8")
dial=e.split("VoidcoreEditor::Dial::Dial(",1)[1].split("void VoidcoreEditor::OrbPad::apply(",1)[0]
fader=e.split("VoidcoreEditor::Fader::Fader(",1)[1].split("void VoidcoreEditor::fader(",1)[0]
main=e.split("void VoidcoreEditor::buildCore(){",1)[1].split("void VoidcoreEditor::buildDeeper(){",1)[0]
assert "setSize(1440,1080);" in e
assert "setFixedAspectRatio(4.0/3.0)" in e
assert "setMouseDragSensitivity(480)" in dial
assert "setVelocityModeParameters(.24,2,0.,true,ModifierKeys::shiftModifier)" in dial
assert "setDoubleClickReturnValue(true" in dial
assert "setScrollWheelEnabled(true)" in dial
assert "isMouseOverOrDragging()" in dial
assert "displayedNormal+=(norm-displayedNormal)" in dial
for phrase in ('parameterID=="rate"','parameterID=="tuneA"','parameterID=="toneLow"','parameterID=="attack"','parameterID=="pan"','getMaximum()<=1.001'):
    assert phrase in dial, f"Missing readable unit format: {phrase}"
assert "getValue()" in dial and "displayedNormal" in h
assert "for(int tick=0;tick<=10;++tick)" in dial
assert "String::formatted(\"%+.1f dB\"" in dial
assert "stripY=h*.811f" in dial
assert "setMouseDragSensitivity(480)" in fader
assert "setVelocityModeParameters(.24,2,0.,true,ModifierKeys::shiftModifier)" in fader
assert "fillRoundedRectangle(x-11.f,cy-19.f,22.f,38.f" in fader
assert "std::max(1.f,x-left)" in fader
assert "height=(y>=620.f&&y<745.f?1.16f" in e
assert 'fader("performanceMod",865,794,233,38,Core);' in main
assert 'add("performanceMod","Performance Mod Wheel",0.f,1.f,0.f)' in p
assert 'p.performanceMod=get("performanceMod")' in p
assert "buildCore();setPage(Core);" in e
assert "node.onPage==20+focusLfo" in e
for key in ("morph","character","grain","delayMix","reverbMix","toneLow","cutoff"):
    assert 'knob("'+key+'"' in main
assert re.search(r"Colour::fromRGB\((\d+),\1,\1\)",dial)
print("PASS: enlarged, precision-dragged real knobs and faders, damped visual needles, permanent numeric units, grayscale UI")
