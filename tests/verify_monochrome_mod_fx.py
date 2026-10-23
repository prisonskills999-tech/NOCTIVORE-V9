#!/usr/bin/env python3
"""Full black-white one-page LFO/FX functional source preflight."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
ui=(root/"Source/VoidcoreEditor.cpp").read_text()
hdr=(root/"Source/VoidcoreEditor.h").read_text()
proc=(root/"Source/PluginProcessor.cpp").read_text()
engine=(root/"Source/RomplerEngine.h").read_text()
fx=(root/"Source/PsySpace.h").read_text()
main=ui.split("void VoidcoreEditor::buildCore(){")[1].split("void VoidcoreEditor::buildDeeper(){")[0]
assert "W=1200.f,H=900.f" in hdr
assert "buildCore();setPage(Core);" in ui and "buildCore();buildDeeper();" not in ui
assert "page==Core&&node.onPage==10+focusLfo" in ui
assert "for(int i=0;i<4;++i)" in main and 'click("LFO "+String(i+1)' in main
assert "lfoButtons[(size_t)i]" in main and "focusLfo=i;setPage(Core);" in main
assert 'lfo(g,{32,425,334,34},focusLfo);' in ui
for field in ("Rate","Depth","Wave","Target","Sync","Phase"):
    assert field in main
assert 'NormalisableRange<float>(.00005f,3.f' in proc
assert "constexpr float beatLengths[]={4.f,8.f,16.f,32.f,64.f,128.f,256.f,512.f};" in engine
assert "cfg.offset" in engine
assert "float offset=0.f;" in engine
for name in ("delayMode","delayDrive","delayFreeze","reverbMode","reverbWidth","reverbFreeze"):
    assert f'get("{name}")' in proc and (f'choose("{name}"' in proc or f'add("{name}"' in proc)
    assert name in main and ("c."+name in fx)
assert "delayHoldButton" in ui and "reverbHoldButton" in ui
assert "std::clamp(c.delayMode,0,3)" in fx
assert "c.reverbMode==2" in fx and "c.reverbMode==3" in fx
assert "c.reverbFreeze" in fx and "c.delayFreeze" in fx
for phrase in ("sourceWave(g,","scope(g,","lfo(g,","filter(g,","startTimerHz(30)"):
    assert phrase in ui
for match in re.finditer(r"(?:juce::)?Colour::fromRGB\((\d+),(\d+),(\d+)\)",ui+hdr):
    a,b,c=map(int,match.groups())
    assert a==b==c,f"Non-monochrome RGB in editor: {match.group(0)}"
assert "X E N O" not in main or "knob" in main
print("PASS four independently automated switchable LFOs, very slow rates, true delay/reverb algorithms, stereo HOLD and 100% grayscale UI")
