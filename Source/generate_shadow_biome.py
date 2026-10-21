#!/usr/bin/env python3
"""NOCTIVORE 10.3 FOREST SIGNALS: 200 original, soft-attack 48k WAV recordings.
No samples, code or recordings from G-Sonique are used. No broadband noise layer.
Tonal wet bleeps, breathy vowels, organic FM, insects and evolving mycelium.
"""
from pathlib import Path
import re
import numpy as np
from scipy import signal
from scipy.io import wavfile
SR=48000
NOTES=(36,48,60,72)
FAMILIES=('Glowworm Lanterns','Moss Pool Bubbles','Root Modem Choir','Whispering Fern Throat',
'Amber Droplet Swarm','Wet Circuit Larvae','Mycelium Signal Relay','Saproot Syllables',
'Hollow Forest Choir','Alien Pond Conversations','Bark Membrane Organ',
'Luminous Fungal Glass','Gurgling Mud Springs','Swamp Microtone Body',
'Canopy Lattice','Spiral Leaf Organ','Polymer Reed Colony','Bacterial Call and Response',
'Gliding Root Pools','Pollen Orbit','Vowel Beetle Village','Underground Telegraph',
'Sinebent Fireflies','Wormhole Sapflow','Underbrush Telepathy')
def bank_info():
    txt=(Path(__file__).parent/'BankCatalog.h').read_text(encoding='utf8')
    rows=re.findall(r'\{"([^"]+)","([^"]+)",(\d+),(true|false)\}',txt)
    assert len(rows)==50, f'{len(rows)} banks instead of 50'
    return rows
def create(bank,note,kind=0,style=None):
    style=bank%25 if style is None else style
    duration=8.1+.32*(bank%5)
    t=np.arange(int(round(duration*SR)),dtype=np.float64)/SR
    f=440.0*2.0**((note-69)/12)
    phi=((1147+29*bank+71*note)*.61803398875%1)*2*np.pi
    slow=np.sin(2*np.pi*(.028+.005*(bank%7))*t+phi)
    tide=np.sin(2*np.pi*(.105+.012*(bank%8))*t+.57*phi)
    curl=np.sin(2*np.pi*(.31+.031*(bank%9))*t+1.23*phi)
    phase=2*np.pi*f*np.cumsum(1+.009*(.58*slow+.28*tide+.14*curl))/SR
    def o(r=1,w=.3):
        return np.sin(phase*r+w*(slow+.4*curl))
    def fm(c=1,m=.5,index=1.):
        return np.sin(phase*c+index*(1+.24*tide)*np.sin(phase*m+.8*slow))
    def pulse(speed=1.4,offset=0):
        env=.5+.5*np.sin(2*np.pi*speed*t+1.3*tide+slow+offset+phi)
        return .24+.76*env**2
    def cl(rs,ws,motion=1.):
        value=np.zeros_like(t)
        for i,(r,w) in enumerate(zip(rs,ws)):
            value+=w*np.sin(phase*r+motion*(1+i*.2)*slow+i*1.2)
        return value
    # Each bank has its own topology instead of one shared harmonic drone.
    if style==0:
        x=.27*o(.5,1.8)+(.46*fm(2.07,.91,.68)+.19*o(3.07,1.3))*pulse(2.1)+.16*o(1.01,1.9)
    elif style==1:
        x=(.48*fm(.52,.237,1.2)+.34*o(1.02,3.2))*pulse(.76)+.24*o(.249,1.4)
    elif style==2:
        x=.3*o(.48,1.3)+.35*fm(1.98,2.31,1.05)*pulse(1.74)+.25*fm(3.03,.618,.45)*pulse(3.3,2.)
    elif style==3:
        x=(.47*fm(.501,.29,1.8)+.34*o(1.,3.8)+.23*o(1.98,3.))*(.73+.27*slow*tide)
    elif style==4:
        x=cl((.25,.5,1.,1.5,2.02),(.15,.27,.33,.24,.12),1.9)+.28*fm(2.,.753,1.4)*pulse(1.08)
    elif style==5:
        x=.48*fm(1.17,.457,1.6)+.26*fm(2.,.87,.73)*pulse(2.4)+.2*o(.502,2.2)
    elif style==6:
        x=.32*o(.498,.9)+.3*fm(1.01,1.49,.84)*pulse(1.3)+.23*fm(2.54,.7,1.)*pulse(3.1,1.7)
    elif style==7:
        x=.38*o(.501,1.6)+(.28*fm(1.,.286,1.2)+.25*o(2.,2.3))*pulse(1.4)
    elif style==8:
        x=cl((.33,.502,.99,1.998,3.,4.03),(.22,.25,.32,.2,.16,.1),2.1)*(.82+.18*slow)
    elif style==9:
        x=.28*o(.5,.9)+.39*fm(1.74,.63,1.34)*pulse(1.9)+.26*o(2.37,3.1)*pulse(2.71,2.8)
    elif style==10:
        x=cl((.251,.499,1.004,2.01,2.98),(.14,.30,.34,.22,.16),2.2)+.18*fm(1.46,.32,.9)
    elif style==11:
        x=.23*o(.51,2)+.23*o(1.013,2.1)+.38*fm(2.07,.318,.85)*pulse(.68)+.22*o(3.01,1.8)
    elif style==12:
        x=(.42*fm(.502,.681,1.)+.31*o(1.501,2.8))*pulse(.58)+.19*fm(2.08,1.46,.78)*pulse(2.03)
    elif style==13:
        x=.26*o(.249,1.1)+.34*fm(1.01,.49,1.63)+.3*fm(1.49,.237,1.1)*pulse(1.03)
    elif style==14:
        x=cl((.51,1.,1.49,2.,2.51),(.17,.32,.24,.19,.12),2.6)*(.7+.3*slow**2)
    elif style==15:
        x=.31*o(.499,2.)+.31*o(.503,3.)+.24*o(1.009,2.2)+.18*fm(2.02,.511,.9)
    elif style==16:
        x=(.55*o(.997,2.9)+.21*o(2.003,4.1))*pulse(.38)+.24*fm(.5,.291,1.4)
    elif style==17:
        x=.36*fm(.74,.29,1.4)*pulse(.81)+.34*fm(2.07,.71,.77)*pulse(1.43,.7)+.2*o(.499,.8)
    elif style==18:
        x=.42*fm(.502,1.106,1.1)+.31*fm(1.25,.379,.89)*pulse(1.13)+.22*o(.251,2.2)
    elif style==19:
        x=cl((.496,1.001,1.993,3.013,4.01),(.14,.34,.23,.16,.08),1.6)+.24*fm(3.03,1.07,.56)*pulse(2.5)
    elif style==20:
        x=(.33*fm(.5,.683,1.8)+.39*fm(1.,.327,1.2))*pulse(.54)+.24*fm(2.01,.987,.7)*pulse(2.08)
    elif style==21:
        x=.25*o(.25,1.1)+.33*o(.506,2.1)+.3*fm(1.99,.917,1.)*pulse(1.33)+.18*o(1.01,1.3)
    elif style==22:
        x=.22*o(.499,1.2)+.38*o(1.502,1.4)+.3*fm(2.753,.375,1.1)*pulse(2.29)
    elif style==23:
        x=.36*fm(.503,.713,1.45)+.31*o(1.002,3.5)+.26*fm(1.51,1.045,.85)*pulse(.48)
    else:
        x=.24*o(.498,2.4)+.29*fm(1.01,.314,1.45)+.24*fm(2.04,.863,1.4)*pulse(2.2)+.2*o(3.02,2.9)*pulse(.87,2.5)
    if bank>=25:
        x=.74*x+.20*fm(.693+.031*(bank%4),.294+.037*(bank%6),.85+.2*curl)
        x*=.86+.14*np.sin(2*np.pi*(.047+.004*(bank%6))*t+phi)**2
    # No pitch drops or struck resonances at note start. Use a slow, transparent swell.
    x*=(.86+.14*(1+slow*tide)*.5)
    attack=(1-np.exp(-t/.17))
    release=(1-np.exp(-np.maximum(duration-t,0)/.18))
    x*=attack*release
    # Low DC, no noise, gentle high-frequency limit.
    sos=signal.butter(2,(80.0,12500.0),btype='bandpass',fs=SR,output='sos')
    x=signal.sosfilt(sos,x)
    x-=x.mean()
    rms=float(np.sqrt(np.mean(x*x))+1e-12)
    target=.135 if style in (0,2,6,9,17,19,24) else .16
    x*=target/rms
    peak=np.max(np.abs(x))
    if peak>.86:x*=.86/peak
    return np.rint(np.clip(x,-1,1)*32767).astype('<i2')
def main():
    root=Path(__file__).resolve().parent.parent
    banks=root/'Banks';banks.mkdir(parents=True,exist_ok=True)
    entries=bank_info()
    lines=['# NOCTIVORE 10.3 — Forest Signals Source Atlas','',
           '200 original clean-tonal 48 kHz PCM16 recordings with sustained motion.','','| # | Bank | Sound family | Role |','|---:|---|---|---|']
    for bank,(name,category,kind,_) in enumerate(entries):
        for note in NOTES:
            wavfile.write(banks/f'{bank:02d}_{note}.wav',SR,create(bank,note,int(kind)))
        role=('BODY','MICRO-FX','VOICE','MOTION','SPECTRAL SPACE')[bank%5]
        lines.append(f'| {bank:02d} | {name} | {FAMILIES[bank%25]} | {role} |')
        if bank%10==9:print(f'Generated {bank+1}/50 banks',flush=True)
    (banks/'MANIFEST.txt').write_text('NOCTIVORE Forest Signals 10.3 - 200 original 48k samples.\n')
    lines+=['','No proprietary G-Sonique audio, data or source files are used.']
    (root/'SOUND_ATLAS.md').write_text('\n'.join(lines)+'\n',encoding='utf8')
    print('Completed 200 original forest WAV samples',flush=True)
if __name__=='__main__':main()
