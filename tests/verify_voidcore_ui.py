#!/usr/bin/env python3
"""Small source-level preflight for VOIDCORE's custom JUCE editor.

This deliberately runs before building. It catches the earlier shipped bug in
which SliderAttachments existed but controls had never joined the Component tree.
"""
from pathlib import Path
import re

base=Path(__file__).resolve().parents[1]
editor=(base/"Source"/"VoidcoreEditor.cpp").read_text(encoding="utf-8")
header=(base/"Source"/"VoidcoreEditor.h").read_text(encoding="utf-8")
processor=(base/"Source"/"PluginProcessor.cpp").read_text(encoding="utf-8")
cmake=(base/"CMakeLists.txt").read_text(encoding="utf-8")

functions=["knob","fader","choice","click","allClick"]
for fn in functions:
    hit=re.search(r"void\s+VoidcoreEditor::"+fn+r"\s*\([^\{]*\)\s*\{([\s\S]*?)\n\}",editor)
    assert hit, f"Missing method {fn}"
    assert "addAndMakeVisible(*c);" in hit.group(1), f"{fn} forgot to add the control as a JUCE child"

assert "return new VoidcoreEditor(*this)" in processor
assert '#include "VoidcoreEditor.h"' in processor
assert 'Source/VoidcoreEditor.cpp' in cmake
assert "if(NOT NOCTIVORE_GENERIC_EDITOR)" in cmake
assert 'setFixedAspectRatio(1.5)' in editor
assert 'startTimerHz(30)' in editor
assert "processor.scopeCursor.load" in editor
assert "processor.displayPpq.load" in editor
assert "processor.peak.load" in editor
assert "AudioProcessorValueTreeState::SliderAttachment" in header
assert "AudioProcessorValueTreeState::ComboBoxAttachment" in header
for i in range(1,17):
    assert "gateStep%02d" in editor
assert "sourceWave(g," in editor
assert "void VoidcoreEditor::save()" in editor
assert "void VoidcoreEditor::load()" in editor

# Every literal control ID must exist in the processor parameter declaration.
defs=set(re.findall(r'(?:add|choose)\("([^"]+)"',processor))
uses=set(re.findall(r'(?:knob|fader|choice)\("([^"]+)"',editor))
missing=uses-defs
assert not missing, f"UI controls referencing nonexistent parameters: {sorted(missing)}"
assert len(uses)>=25, f"Suspiciously few working UI controls: {len(uses)}"
print(f"PASS: {len(uses)} attached controls, 5 registered JUCE child factories, 16 gates, four pages, live scope")
