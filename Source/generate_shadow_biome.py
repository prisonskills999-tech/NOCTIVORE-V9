#!/usr/bin/env python3
"""NOCTIVORE v10 - 200 original NOISE-FREE 48 kHz soundscape multisamples.

25 distinct, deterministically synthesized physical/organic/fantastical recipes.
No stochastic broadband noise is rendered. Random numbers choose fixed voice
parameters only; all audible content is pitched/quasiperiodic resonant material.
No proprietary recordings or samples are used.
"""
from pathlib import Path
import re
import numpy as np
from scipy import signal
from scipy.io import wavfile

SR=48000
NOTES=(36,48,60,72)
FAMILIES=(
 'Subterranean throat', 'Hollow woodwind', 'Liquid membranes', 'Forest clicking',
 'Vowel metamorphosis', 'Bent glass', 'Fungal organ', 'Symbiotic pulse',
 'Molten rubber', 'Alien bird-cry', 'Wormhole resonance', 'Amphibian glottis',
 'Spectral pollen', 'Low ritual horn', 'Psychedelic spiral', 'Cavern harmonium',
 'Cellular squelch', 'Formant polyphony', 'Slow tectonic folds', 'Chitinic trills',
 'Undersea sonar', 'Fractal phasing', 'Subatomic oscillation', 'Ritual bone flute',
 'Organic signal corruption'
)

def bank_info():
    text=(Path(__file__).resolve().parent/'BankCatalog.h').read_text(encoding='utf-8')
    rows=re.findall(r'\{"([^"]+)","([^"]+)",(\d+),(true|false)\}',text)
    assert len(rows)==50, len(rows)
    return [(name,category,int(kind)) for name,category,kind,_ in rows]

def lpf(x,cutoff):
    return signal.sosfilt(signal.butter(2, float(cutoff), fs=SR, output='sos'), x)

def create(bank,note,kind,style):
    rng=np.random.default_rng(3907+bank*157+note*31)
    duration=7.0+0.35*(bank%5)  # 7 - 8.4s sustained, loopable
    n=int(round(duration*SR))
    t=np.arange(n,dtype=np.float64)/SR
    f=440.0*2.0**((note-69)/12)
    phi=float(rng.uniform(0,2*np.pi))
    slow=.036+.007*(bank%7)
    middle=.13+.012*(bank%9)
    quick=.42+.061*(bank%6)
    q=np.sin(2*np.pi*slow*t+phi)
    p=np.sin(2*np.pi*middle*t+.83*phi)
    z=np.sin(2*np.pi*quick*t+1.7*phi)
    squirm=np.sin(2*np.pi*(.81+.17*(bank%8))*t+1.35*q)
    flutter=(.5+.5*np.sin(2*np.pi*(2.3+.2*(bank%8))*t+q))**6
    # Highly stable band-limited quasi-harmonic phase, no noise / no hiss.
    glide=1.+.008*q+.0023*p
    theta=2*np.pi*f*np.cumsum(glide)/SR
    def o(r=1.0, bend=0.0, skew=0.0):
        return np.sin(theta*r+bend*q+skew*np.sin(.31*theta+.7*p))
    def fm(a=1.,b=1.5,idx=1.6):
        return np.sin(theta*a+idx*(1+.28*p)*np.sin(theta*b+.8*q))
    def phasecluster(ratios, weights, motion=0.4):
        out=np.zeros_like(t)
        for i,(ratio,w) in enumerate(zip(ratios,weights)):
            out+=w*np.sin(theta*ratio+motion*(i+1)*q+(i*.35)*p)
        return out
    # Recipes are deliberately non-equivalent: different spectral shapes,
    # time envelopes and inharmonic relationships, not the same drone+noise.
    if style==0:  # deep false-vocal creature glottis
        x=.54*fm(.49,1.03,2.3)+.34*o(.251,2.4)+.26*fm(1.01,.71,1.6)
        x*=.66+.34*np.maximum(0,squirm)**2
    elif style==1:  # hollow pipes and filtered harmonic modes
        x=phasecluster((.498,1,1.996,2.99,4.04),(.62,.26,.17,.12,.07),1.1)
        x+=.19*o(.746,3.2)
    elif style==2:  # slippery stretched membranes
        x=.45*fm(.502,.82,3.5)+.33*fm(1.001,.33,1.8)+.19*o(.255,3.4)
        x*=.52+.48*(.5+.5*np.sin(2*np.pi*.54*t+1.8*q))**2
    elif style==3:  # dry pitched insect clacks / distinct gap shapes
        x=(.32*o(1.44,1.5)+.36*fm(2.53,1.21,2.4))*flutter+.18*o(.51,2)
        x*=.72+.28*np.abs(p)
    elif style==4:  # multi-vowel choir, no breath noise
        x=np.zeros_like(t)
        # resonant sinusoidal harmonic formants glide up/down independently
        F1=430+460*(.5+.5*q);F2=1150+780*(.5+.5*p)
        for h in range(1,13):
            weight=.20+1.7*np.exp(-((f*h-F1)/220)**2)+1.1*np.exp(-((f*h-F2)/340)**2)
            x+=(weight/(h**.72))*o(h,.14*h)
        x*=.15
    elif style==5:  # bent glass, harmonics with short inharmonic skirts
        x=phasecluster((.507,.998,1.995,2.74,3.97),(.28,.35,.22,.18,.11),1.8)
        x+=.23*fm(1.002,2.23,2.1)
    elif style==6:  # dense, morphing fungal organ
        x=.28*o(.251,1.9)+.29*o(.499,2.1)+.29*o(1.001,1.8)+.15*o(2.02,4.3)
        x*=.64+.36*(.5+.5*q)**2
    elif style==7:  # purposeful 5:7 sympathetic rhythm
        e=(.2+.8*(.5+.5*np.sin(2*np.pi*1.33*t+.3*p))**4)
        x=(.47*fm(.489,1.22,2.7)+.31*o(1.003,3.3))*e+.19*o(.246)
    elif style==8:  # rubber mouth / resonant wah
        x=.59*fm(.72,.315,3.9)+.28*fm(1.48,.412,1.4)+.16*o(.50,3.1)
    elif style==9:  # uncanny pitched bird glissandi but no note-sequencing
        x=.4*fm(1.97,.727,2.6)+.37*o(.991,1.6,1.2)+.21*o(.493,2.3)
        x*=.25+.75*flutter
    elif style==10: # phase-based wormhole / tiny beating dissonances
        x=phasecluster((.248,.5,.504,1.003,1.013,2.017),(.22,.23,.28,.27,.17,.13),1.7)
    elif style==11: # slow wet glottal deformation
        x=.51*fm(.497,1.313,4.6)+.25*fm(.999,.405,2.4)+.18*o(.248)
        x*=.38+.62*(.5+.5*np.sin(2*np.pi*.69*t+1.1*q))**3
    elif style==12: # pollen spectral motes / quiet fast sine partials
        x=phasecluster((.498,.753,1.002,1.49,2.011,2.51,3.06),(.22,.18,.31,.12,.13,.11,.07),2.0)
        x*=.64+.36*np.abs(z)
    elif style==13: # ritual subhorn, massive body
        x=.53*o(.251,2)+.39*o(.501,1.5)+.21*fm(.998,.75,1.4)
    elif style==14: # harmonic psychedelic spiral, no noise bed
        x=.5*fm(.749,1.11,2.9)+.31*fm(1.492,.612,1.2)+.22*o(.249,1.4)
        x*=.65+.35*np.sin(2*np.pi*.23*t+1.4*q)**2
    elif style==15: # cavern harmonium, moving resonances
        x=phasecluster((.498,.998,1.492,2.005,2.997),(.44,.33,.22,.18,.09),1.25)
        x+=.17*fm(1.,.27,.72)
    elif style==16: # sudden elastic cell contractions / non-melodic
        e=(.12+.88*(.5+.5*np.sin(2*np.pi*1.92*t+1.7*p))**8)
        x=.57*fm(1.05,.87,3.4)*e+.25*o(.503,1.8)+.17*o(.252)
    elif style==17: # dissonant formant polyphony
        x=.36*fm(.998,.249,1.3)+.38*fm(1.498,.971,1.7)+.25*fm(.502,.722,2.1)
        x*=.80+.20*q
    elif style==18: # ultra-low geological fold
        x=.41*o(.125,2.1)+.37*o(.251,2.3)+.21*fm(.504,.619,3.1)
    elif style==19: # trilling wing made entirely of quasi-periodic FM
        e=(.24+.76*(.5+.5*np.sin(2*np.pi*(3.4+.1*(bank%7))*t+q))**3)
        x=.33*o(.499,2.7)+(.27*fm(2.31,1.603,2.6)+.31*o(3.15,2.1))*e
    elif style==20: # organic sonar, rising beating circles
        x=.48*o(.503,3.5)+.22*fm(1.01,.37,2.8)+.22*o(2.01,3.)
        x*=.3+.7*(.5+.5*np.sin(2*np.pi*.42*t+.6*p))**4
    elif style==21: # fractal, phase-offset harmonic lattice
        x=phasecluster((.249,.51,.766,.997,1.502,2.048,3.105),(.13,.30,.20,.26,.16,.15,.08),2.4)
    elif style==22: # unstable cellular oscillation, not noise
        x=.35*fm(.482,1.812,3.2)+.34*fm(.992,.421,2.1)+.24*o(.245,1.7)
        x*=.50+.50*(.5+.5*z)**4
    elif style==23: # bone/wood flute with warm overblown spectral response
        x=.51*o(.998,1.9)+.22*o(1.994,3.1)+.12*o(3.007,3.7)+.27*o(.501,1.6)
    elif style==24: # enigmatic rhythmic transmission made of structured oscillations
        e=.25+.75*(.5+.5*np.sin(2*np.pi*(1.67+.07*(bank%5))*t+q+2*p))**7
        x=.37*fm(1.001,.38,3.4)*e+.35*fm(.511,.816,1.8)+.22*o(.256,2.6)
    else:raise ValueError(style)
    # Keep natural, bank-specific but broadband-noise-free dynamics.
    swell=.80+.20*np.sin(2*np.pi*(.029+.004*(bank%7))*t+phi+.45*q)**2
    x*=swell
    fade=(1-np.exp(-t/.018))*(1-np.exp(-(duration-t)/.07))
    x=lpf(x*fade, min(12500.,max(6400., 8400.+1300.*(bank%5))))
    x-=np.mean(x)
    rms=np.sqrt(np.mean(x*x))+1e-12
    target=.15 if style in (3,9,12,19,20,24) else .18
    x*=target/rms
    peak=np.max(np.abs(x))
    if peak>.88:x*=.88/peak
    return np.rint(np.clip(x,-1,1)*32767).astype('<i2')

def main():
    root=Path(__file__).resolve().parent.parent
    banks=root/'Banks';banks.mkdir(parents=True,exist_ok=True)
    entries=bank_info()
    lines=['# NOCTIVORE v10 — Noise-Free Xenobiome Source Atlas','',
      '200 original PCM16 WAV recordings, synthesized at 48 kHz without stochastic broadband noise.','',
      '| # | Bank | Organic model | A/B role |','|---:|---|---|---|']
    for bank,(name,category,kind) in enumerate(entries):
        # Covers all 25 types twice, while large sound families stay contrasting.
        model=((bank%25)*11)%25
        for note in NOTES:
            wavfile.write(banks/f'{bank:02d}_{note}.wav',SR,create(bank,note,kind,model))
        role=('BODY','VOICE','MOTION','PUNCTUATION','SPACE')[bank%5]
        lines.append(f'| {bank:02d} | {name} | {FAMILIES[model]} | {role} |')
        if bank%10==9:print(f'Generated {bank+1}/50 noise-free multisample banks',flush=True)
    (banks/'MANIFEST.txt').write_text('NOCTIVORE v10: 200 noise-free original 48 kHz WAVs; 25 synthetic organic models.\n')
    lines+=['','Recommended contrasting pairings: A00+B23, A02+B48, A07+B33, A11+B42, A20+B14, A26+B39.',
            'No G-Sonique proprietary sound libraries or copyrighted recordings are used.']
    (root/'SOUND_ATLAS.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print('Completed 200 noise-free sound sources',flush=True)
if __name__=='__main__':main()
