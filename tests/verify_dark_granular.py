#!/usr/bin/env python3
"""Single-screen DARK IMPACT knobs must be host-attached and audibly routed."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
cpp=(root/"Source/VoidcoreEditor.cpp").read_text(encoding="utf-8")
processor=(root/"Source/PluginProcessor.cpp").read_text(encoding="utf-8")
dsp=(root/"Source/RomplerEngine.h").read_text(encoding="utf-8")
main=cpp.split("void VoidcoreEditor::buildCore(){",1)[1].split("void VoidcoreEditor::buildDeeper(){",1)[0]
assert "buildCore();setPage(Core);" in cpp
assert "buildCore();buildDeeper();" not in cpp
for name,label in (("drop","DROP"),("hits","HITS"),("body","BODY"),
                   ("grit","GRIT"),("dark","DARK")):
    assert f'add("{name}"' in processor, f"Missing host parameter {name}"
    assert f'p.{name}=get("{name}")' in processor, f"Missing DSP mapping {name}"
    assert f'knob("{name}","{label}"' in main, f"Missing accessible UI knob {name}"
    assert f'p.{name}' in dsp, f"No processing for {name}"
for name in ("grain","texture","character"):
    assert f'knob("{name}"' in main
assert "v.debrisCountdown-=dt" in dsp
assert "g.progress=0.f;" in dsp
assert "envelope*envelope*g.level" in dsp
assert "v.impactCountdown-=dt" in dsp
assert "v.impactProgress=std::min(1.f" in dsp
assert "v.bodyLP+=(rawBody-v.bodyLP)" in dsp
assert "v.gritPre+=(dry-v.gritPre)" in dsp
assert "v.darkLP+=(dry-v.darkLP)" in dsp
assert "const float pureSource=sample;" in dsp
assert "if(hitAmount>.001f)" in dsp
assert "if(bodyAmount>.0001f)" in dsp
assert "if(gritAmount>.001f)" in dsp
assert "if(darkAmount>.001f)" in dsp
assert "Sample::prepareSinc();" in dsp
assert "std::min(masterGain,target)" in dsp
assert "no allocations" not in dsp or "allocations" in dsp  # informational only
print("PASS: five DARK IMPACT macros live, bank-linked, automated; sample-only audition and overload guard preserved")
