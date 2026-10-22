#!/usr/bin/env python3
"""Catch regressions that make all VOIDCORE banks share a harsh synthetic layer."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
engine=(root/"Source/RomplerEngine.h").read_text(encoding="utf-8")
processor=(root/"Source/PluginProcessor.cpp").read_text(encoding="utf-8")
editor=(root/"Source/VoidcoreEditor.cpp").read_text(encoding="utf-8")
header=(root/"Source/VoidcoreEditor.h").read_text(encoding="utf-8")

# Fractional ratios applied to a wrapped phase create a discontinuity each
# oscillator cycle, injecting a repeated crackle on every patch.
wrapped=re.findall(r'\bv\.(?:modPhase|fmPhase|motionPhase|cellPhase)\s*\*\s*(?:\d+\.\d+|\.\d+)f',engine)
assert not wrapped, f"Aliasing-prone wrapped phase ratios: {wrapped}"
assert "v.fmAltPhase=wrapPhase(v.fmAltPhase" in engine
assert "v.fmAltPhase+(fmAmount*.7f)*std::sin(v.modPhase)" in engine

# The audition must actually tap the decoded PCM before any added sources,
# filter stages, saturation or shared FX and must stay host-automatable.
assert "const float pureSource=sample;" in engine
assert "const float pureL=pureSource*pureGain" in engine
assert "const float pureR=pureSource*pureGain" in engine
assert "v.smoothSolo+=" in engine
assert "wetL*=1.f-smoothedSoloFx" in engine
assert 'add("morph","Source Crossfade",0,1,0.f)' in processor
assert "p.morphMotion*(.5f*(osc+" in engine
assert "masterGain=std::min(masterGain,target)" in engine
assert "left[i]+=rawL*masterGain" in engine
assert "std::tanh((mixL+wetL)" not in engine
assert 'add("character","XENO Exciter Amount",0.f,1.f,0.f)' in processor
assert 'choose("sourceSolo","Pure Source Audition",{"Normal Mix","Pure Source"})' in processor
assert 'p.character=get("character");p.sourceSolo=get("sourceSolo");' in processor
assert 'knob("character","XENO"' in editor
assert 'click("PURE SOURCE"' in editor and 'set("sourceSolo"' in editor
assert "sourceSoloButton" in header

# No default signal chain is allowed to impose the same wet/distorted layer
# on different bank samples.
for declaration in (
    'add("delayMix","Delay Wet",0,1,0.f)',
    'add("reverbMix","Reverb Wet",0,1,0.f)',
    'add("fmMix","FM Amount",0.f,1.f,0.f)',
    'add("hiFiAir","Hi-Fi Air",0.f,1.f,0.f)',
    'add("trancePulse","Psy Pulse",0.f,1.f,0.f)',
):
    assert declaration in processor, f"Unexpected shared coloration default: {declaration}"
assert "fx.reverbMix*(1.f+.09f*p.macroAtmosphere)" in engine
assert "fx.reverbMix+.09f*p.macroAtmosphere" not in engine
assert "const float g=std::tan(pi*hz*dt);" in engine
assert "float hp=input-lp-q*bp;" not in engine
assert "float smoothedSoloFx=" not in engine  # member is initialized as part of group
assert "smoothedSoloFx=0.f;" in engine
assert "spaceFX.tick(mixL,mixR,wetL,wetR);" in engine
print("PASS: phase-continuous oscillators, dry source audition, neutral defaults, stable filter and optional XENO color")
