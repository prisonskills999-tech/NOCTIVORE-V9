#!/usr/bin/env python3
"""NOCTIVORE v10.6 — original noise-free experimental 48 kHz PCM16 atlas."""
from pathlib import Path
import numpy as np
from scipy.io import wavfile
from scipy import signal
import generate_shadow_biome as original

SR=48000
ROOTS=(36,48,60,72)
ROLES=('Interference mycelium','Glass-skinned microbes','Hollow creature vowels',
'Polytemporal pheromones','Spectral folds','Micro-insect chorus',
'Wet pulse organisms','Fungal transmissions','Deep swamp membranes',
'Fractal forest antennae')

def create(bank,note,category,model):
    base=original.create(bank,note,category,model).astype(np.float64)/32768.
    n=len(base);t=np.arange(n,dtype=np.float64)/SR
    f=440.*2**((note-69)/12)
    seed=(bank*171+note*37)%997
    phase=2*np.pi*f*t
    low=.075+.013*(bank%9);mid=.25+.043*(bank%7)
    drift=np.sin(2*np.pi*low*t+seed*.08)
    slow2=np.sin(2*np.pi*mid*t+seed*.13)
    pulses=.63+2.8*(bank%6)/5.
    phi=2*np.pi*(pulses*t+.17*np.sin(2*np.pi*low*t))
    env=(.5+.5*np.sin(phi+seed*.021))**3
    swell=.55+.45*(.5+.5*np.sin(2*np.pi*(low*.8)*t+seed*.04))
    fm=np.sin(phase*(.61+.13*(bank%5))+(1.1+2.3*(bank%4)/3)*np.sin(phase*(.83+.17*(bank%3))+slow2))
    side=np.sin(phase*(1.7+.31*(bank%4))+.7*drift)
    sub=np.sin(phase*.5+.9*drift)
    if model==0:
        sculpt=.5*fm+.5*np.sin(phase*.41+3.7*drift)*swell
    elif model==1:
        sculpt=.40*sub+.6*side*(.28+.72*env)
    elif model==2:
        sculpt=.44*np.sin(phase*.93+3.1*drift)+.35*np.sin(phase*2.17+2.1*slow2)+.21*fm
        sculpt*=(.45+.55*env)
    elif model==3:
        e2=(.5+.5*np.cos(2*np.pi*(pulses*.71)*t+seed*.13))**4
        sculpt=(.6*fm*env+.4*sub*e2)*swell
    elif model==4:
        sculpt=np.sin(2.1*(.54*fm+.42*np.sin(phase*.29))+1.7*drift)*(.55+.45*slow2**2)
    elif model==5:
        trem=.5+.5*np.sin(2*np.pi*(3.2+(bank%5)*.6)*t+1.2*drift)
        sculpt=.51*fm*(.20+.8*trem*trem)+.49*np.sin(phase*.19+1.7*slow2)
    elif model==6:
        sculpt=.63*fm*env+.37*np.sin(phase*.35+2.5*slow2)*(1.-.6*env)
    elif model==7:
        e2=(.5+.5*np.sin(2*np.pi*pulses*.57*t+1.2*slow2))**3
        sculpt=.5*np.sin(phase*.51+2.4*drift)*env+.5*np.sin(phase*1.43+1.9*slow2)*e2
    elif model==8:
        sculpt=.64*np.sin(phase*.24+2.7*drift)+.36*fm*(.25+.75*env)
    else:
        sculpt=.34*np.sin(phase*.72+4.*drift)+.31*fm+.35*side*(.4+.6*env)
    material=(.47+.14*(bank%4))*base+(.53-.14*(bank%4))*sculpt*.72
    material*=.68+.32*swell
    fadein=1.-np.exp(-t/(.16+.07*(bank%4)))
    fadeout=1.-np.exp(-(t[-1]-t)/.22)
    material*=fadein*fadeout
    sos=signal.butter(2,min(10500.,5700.+(bank%5)*1100),fs=SR,output='sos')
    material=signal.sosfilt(sos,material)
    material-=np.mean(material)
    rms=np.sqrt(np.mean(material**2))+1e-12
    target=.14 if bank%5 in (0,3) else .17
    material*=target/rms
    peak=np.max(np.abs(material))
    if peak>.84:material*=.84/peak
    return np.rint(np.clip(material,-1.,1.)*32767.).astype('<i2')

def main():
    root=Path(__file__).resolve().parent.parent
    out=root/'Banks';out.mkdir(parents=True,exist_ok=True)
    entries=original.bank_info()
    for bank,(_,category,kind) in enumerate(entries):
        for note in ROOTS:
            wavfile.write(out/f'{bank:02d}_{note}.wav',SR,create(bank,note,kind,bank%10))
        if bank%10==9:print('Created',bank+1,'of 50 banks',flush=True)
    (out/'MANIFEST.txt').write_text('NOCTIVORE v10.6 — 200 original 48kHz/PCM16, noise-free experimental organisms.\\n')
    with (root/'SOUND_ATLAS.md').open('w') as outmd:
        outmd.write('# NOCTIVORE v10.6 — ABYSSAL ECOLOGY\\n\\n50 banks / 200 original samples.\\n\\n')
        for idx,(name,_,_) in enumerate(entries):
            outmd.write(f'- {idx:02d} **{name}** — {ROLES[idx%10]}\\n')
        outmd.write('\\nOriginal deterministic synthesis; no copied or third-party recordings.\\n')
    print('All 200 regenerated',flush=True)

if __name__=='__main__':main()
