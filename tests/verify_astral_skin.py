#!/usr/bin/env python3
"""Static preflight for the real Astral Bloom single-screen JUCE VST3 skin."""
from pathlib import Path
import re
root = Path(__file__).resolve().parents[1]
cpp = (root / "Source/VoidcoreEditor.cpp").read_text(encoding="utf-8")
header = (root / "Source/VoidcoreEditor.h").read_text(encoding="utf-8")
processor = (root / "Source/PluginProcessor.cpp").read_text(encoding="utf-8")
assert "void VoidcoreEditor::drawCore(" in cpp
main = cpp.split("void VoidcoreEditor::buildCore(){", 1)[1].split("void VoidcoreEditor::buildDeeper(){", 1)[0]
draw = cpp.split("void VoidcoreEditor::drawCore(", 1)[1].split("void VoidcoreEditor::drawDeep(", 1)[0]
for name in ("OSC  /  SOURCE", "FILTER", "MOTION  /  LFO", "SPACE  /  FX",
             "ENVELOPE  /  AMP", "OUTPUT", "LIVING  /  MACROS"):
    assert name in draw, f"Missing visible section: {name}"
for name in ("sourceWave(g,", "filter(g,", "lfo(g,", "env(g,", "effects(g,",
             "scope(g,", "visualEye(g,", "drawKeyboardShell(g,"):
    assert name in draw, f"Missing live drawing: {name}"
for signature in ("void VoidcoreEditor::OrbPad::mouseDown", "void VoidcoreEditor::OrbPad::mouseDrag",
                  "void VoidcoreEditor::OrbPad::mouseUp", "void VoidcoreEditor::soundSphere",
                  "void VoidcoreEditor::Dial::paint", "void VoidcoreEditor::drawOrbitalFrame"):
    assert signature in cpp
assert 'p->beginChangeGesture()' in cpp
assert 'p->setValueNotifyingHost(' in cpp
assert 'p->endChangeGesture()' in cpp
assert "struct OrbPad final" in header
assert "filterModeButtons" in header
assert "MidiKeyboardComponent" in cpp
assert "scopeCursor.load" in cpp and "peakLeft" in cpp and "peakRight" in cpp
ids = re.findall(r'\b(?:knob|fader|choice)\("([^"]+)"', main)
assert len(ids) >= 35, f"Expected 35+ physically attached controls, got {len(ids)}"
defs = set(re.findall(r'(?:add|choose)\("([^"]+)"', processor))
missing = set(ids) - defs
assert not missing, f"Unrecognized host parameters: {sorted(missing)}"
assert 'set("filterMode",(float)i)' in main
assert 'shift("bank",-1)' in main and 'shift("bankB",1)' in main
assert 'state.getParameter("biomeDepth")' in cpp
assert 'state.getParameter("morph")' in cpp
assert "node.control->repaint()" in cpp and "startTimerHz(30)" in cpp
print(f"PASS Astral: {len(ids)} attached real controls, central XY orb, interactive filter, live sound visualizers")
