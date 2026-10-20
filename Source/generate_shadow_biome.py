#!/usr/bin/env python3
"""NOCTIVORE V9 original, fully procedural 48 kHz PCM16 source collection.
50 bank identities, 4 playable pitch zones, 15 distinct synthesis designs.
Run from the project tree with numpy + scipy installed. Never uses G-Sonique data.
"""
from __future__ import annotations
from pathlib import Path
import re
import numpy as np
from scipy import signal
from scipy.io import wavfile

SR = 48000
NOTES = (36, 48, 60, 72)
STYLES = ('hollow', 'vowel', 'swarm', 'liquid', 'phase', 'wind', 'ritual',
          'crackle', 'underwater', 'choir', 'radiation', 'elastic', 'crystalline',
          'tape', 'spores')
# Category-to-family mapping, then deliberate per-bank divergences.
KIND_STYLES = {
 0: (0, 5, 13), 1:(3, 4, 11), 2:(6, 8, 12), 3:(2, 7, 14),
 4:(1, 9, 13), 5:(12, 6, 0), 6:(10, 5, 0), 7:(14, 3, 8),
 8:(11, 10, 7), 9:(6, 7, 2)
}

def bank_info():
 text=(Path(__file__).resolve().parent/'BankCatalog.h').read_text()
 rows=re.findall(r'\{"([^"]+)","([^"]+)",(\d+),(true|false)\}',text)
 assert len(rows)==50,len(rows)
 return [(name,cat,int(kind)) for name,cat,kind,_ in rows]

def lp(x,f):
 f=float(np.clip(f,55,SR*.40))
 return signal.sosfilt(signal.butter(2,f,fs=SR,output='sos'),x)

def bp(x,low,high):
 a=float(np.clip(low,70,SR*.37));b=float(np.clip(high,a+150,SR*.46))
 return signal.sosfilt(signal.butter(2,[a,b],btype='band',fs=SR,output='sos'),x)

def gen(bank,note,kind,style):
 r=np.random.default_rng(0x1234ab+bank*5381+note*2399)
 duration=6.7+(.29*(bank%7))  # 6.7–8.44 seconds, all loopable
 n=int(SR*duration)
 t=np.arange(n,dtype=np.float64)/SR
 hz=440.*(2.**((note-69)/12))
 a=.018+.009*(bank%8)
 b=.066+.007*(bank%11)
 c=.15+.031*(bank%9)
 slow=np.sin(2*np.pi*a*t+bank*.63)
 middle=np.sin(2*np.pi*b*t+note*.11+bank*.17)
 fast=np.sin(2*np.pi*c*t+bank*.11)
 wobble=.012*slow+.005*middle  # natural pitch drift, no melodies
 phase=2*np.pi*hz*(t+np.cumsum(wobble)/SR)
 noise=r.standard_normal(n)
 filtered=lp(noise,2500+bank%8*750)
 hot=bp(noise,700+bank%6*320,7200-bank%5*350)
 breath=bp(noise,200+bank%4*110,1600+bank%6*210)
 # Different physical / spectral models, rather than different envelopes on 1 oscillator.
 if style==0: # hollow acoustic wind organ
  x=.55*np.sin(phase*.50+1.8*slow)+.24*np.sin(phase*.996+.9*middle)
  x+=.11*np.sin(phase*2.0+2.3*slow)+.14*breath*(.55+.45*middle)
 elif style==1: # formant-morphing voices
  x=np.zeros(n)
  form1=460+570*(.5+.5*slow);form2=1190+550*(.5+.5*middle)
  for h in range(1,17):
   fh=hz*h
   weight=.10 + 1.35*np.exp(-((fh-form1)/270.)**2)+.9*np.exp(-((fh-form2)/430.)**2)
   x+=weight*np.sin(phase*h+.12*h*middle)/np.sqrt(h)
  x*=.14
  x+=.18*breath*(.45+.55*(slow+1)/2)
 elif style==2: # forest insect / raspy swarms
  cr=np.sin(phase*(2.5+.13*(bank%6)) + (1.2+.6*middle)*np.sin(phase*.735+slow))
  chatter=np.sin(2*np.pi*(2.1+.22*(bank%8))*t+2.2*slow)**8
  x=.26*np.sin(phase*.493)+.52*cr*(.15+.85*chatter)+.23*hot*(.25+.75*chatter)
 elif style==3: # viscous liquid membrane
  subphase=phase*(.49+.014*(bank%5))
  x=.28*np.sin(subphase+4.5*slow)+.31*np.sin(phase+.9*np.sin(subphase*.53+middle))
  x+=.29*np.sin(phase*1.50+2.4*np.sin(subphase*.77+slow))
  x+=.13*filtered*(.45+.55*np.abs(fast))
 elif style==4: # industrial FM shape, but smooth not bell-like
  x=.53*np.sin(phase + (1.6+1.6*slow)*np.sin(phase*.738+middle))
  x+=.29*np.sin(phase*.502+.9*np.sin(phase*1.79+slow))+.13*hot
 elif style==5: # moving spectral air, distinct unpitched texture
  x=.13*np.sin(phase*.253+2.5*slow)
  x+=.41*bp(noise,650+130*(bank%6),4200+200*(bank%7))*(.55+.45*np.abs(slow*middle))
  x+=.27*bp(noise,1900,7500)*(.35+.65*np.abs(middle))
 elif style==6: # ritual bowed wood / skin friction
  x=.43*np.sin(phase*.507+1.8*slow)+.19*np.sin(phase*.752+1.1*middle)
  x+=.24*bp(noise,170,1650)*(.32+.68*(.5+.5*fast)**3)
  x+=.14*hot*(.4+.6*np.abs(slow))
 elif style==7: # fragile, irregular noise cracks and rustles
  carrier=.15*np.sin(phase*.503+slow)+.16*np.sin(phase*1.012+2.1*middle)
  edge=np.maximum(0,np.sin(2*np.pi*(1.1+.19*(bank%9))*t+fast))**10
  x=carrier+.52*hot*(.05+.95*edge)+.22*filtered*np.maximum(0,middle)**3
 elif style==8: # submerged organic bubbling, no pitched runs
  bubble=np.sin(phase*.95+(1.8+3*middle)*np.sin(phase*.51))
  pulse=np.maximum(0,np.sin(2*np.pi*(.6+.07*(bank%9))*t+3*slow))**6
  x=.18*np.sin(phase*.25+slow)+.40*bubble*(.17+.83*pulse)
  x+=.21*bp(noise,380,2900)*pulse
 elif style==9: # haunted tape choir
  x=.21*np.sin(phase*.997+.75*slow)+.24*np.sin(phase*1.502+.93*middle)
  x+=.29*np.sin(phase*.502+2.1*slow)+.11*np.sin(phase*2.005+fast)
  x+=.17*bp(noise,580,2700)*( .4 + .6*np.abs(slow) )
 elif style==10: # radioactive hiss / deep sci-fi transmission
  x=.18*np.sin(phase*.267+3*slow)+.17*np.sin(phase*.873+4*middle)
  interference=np.sin(2*np.pi*(.5+.16*(bank%7))*t+fast)
  x+=.50*bp(noise,1100,8600)*(.3+.7*(.5+.5*interference)**2)
 elif style==11: # elastic electric squelch, no melodic line
  x=.42*np.sin(phase + (1.+2.2*np.abs(slow))*np.sin(phase*.37+middle))
  x+=.2*np.sin(phase*.49+2*np.sin(phase*.21+fast))
  x+=.17*lp(noise,1900)*( .6 + .4*fast )
 elif style==12: # softer crystalline glass shimmer, sparse
  x=.29*np.sin(phase*.99+slow)+.14*np.sin(phase*2.03+middle)
  x+=.19*np.sin(phase*3.07+3.2*slow)+.20*np.sin(phase*4.13+2*middle)
  x+=.13*bp(noise,2800,9600)*(.3+.7*(.5+.5*middle))
 elif style==13: # degraded tape / wowing transmission
  tape=.02*slow+.021*np.sin(2*np.pi*.14*t+middle)
  x=.37*np.sin(phase*.75+tape*phase*.09)+.32*np.sin(phase*1.5+2.3*slow)
  x+=.18*bp(noise,390,4500)*(.65+.35*np.abs(middle))
 elif style==14: # spores / reactive forest clusters
  x=.20*np.sin(phase*.503+slow)
  spores=np.maximum(0,np.sin(2*np.pi*(1.15+.19*(bank%8))*t+1.6*slow))**8
  x+=.34*np.sin(phase*1.77+(1.7+1.3*fast)*np.sin(phase*.91))*spores
  x+=.30*bp(noise,1400,5900)*(.18+.82*spores)
 else: raise ValueError(style)
 # Different trajectories across every bank, not a universal pulsing drone.
 envelope=(.62+.38*np.sin(2*np.pi*(.043+.007*(bank%8))*t+bank*.27+slow)**2)
 if bank%5==0: envelope=.35+.65*np.maximum(0,np.sin(2*np.pi*(.22+.016*(bank%7))*t+slow))**2
 if bank%5==3: envelope=.62+.38*np.abs(middle*fast)
 x*=envelope
 # Very short fade in/out to preserve looping. Engine crossfades its loop region.
 fade=(1-np.exp(-t/.025))*(1-np.exp(-(duration-t)/.05))
 x=lp(x*fade,16000 if style not in (6,8,9) else 11000)
 x-=np.mean(x)
 rms=np.sqrt(np.mean(x*x))+1e-12
 loud={2:.15,5:.14,7:.16,10:.15,12:.16}.get(style,.185)
 x*=loud/rms
 peak=np.max(np.abs(x));
 if peak>.89:x*=.89/peak
 return np.rint(np.clip(x,-1,1)*32767).astype('<i2')

def main():
 root=Path(__file__).resolve().parent.parent
 banks=root/'Banks';banks.mkdir(parents=True,exist_ok=True)
 catalog=bank_info()
 report=['# V9 Shadow Biome — Original Sound Banks','',
         '15 complementary synthesis models; 50 distinct bank identities, four key zones (C2/C3/C4/C5) each.','',
         '| # | Bank | Source character | Layering role |','|---:|---|---|---|']
 for bank,(name,category,kind) in enumerate(catalog):
  variant=(bank//7+bank%3)%3
  style=KIND_STYLES[kind][variant]
  # Promote diversity within large forest/vocal categories
  if bank in (16,25,34,39,42,43,48):style=[2,5,8,11,13,14,7][(16,25,34,39,42,43,48).index(bank)]
  for note in NOTES:
   wavfile.write(banks/f'{bank:02d}_{note}.wav',SR,gen(bank,note,kind,style))
  role=['body','texture','air','motion','transient'][bank%5]
  report.append(f'| {bank:02d} | {name} | {STYLES[style]} | {role} |')
  if bank%10==9:print(f'Built {bank+1}/50 fully original banks',flush=True)
 report+=['', '## Contrasting A/B combinations','',
   '- A 00 Obsidian Invocation + B 05 Blackglass Insects — hollow body / prickly swarm',
   '- A 02 Mycelium Choir + B 25 Shadow Biome — voice / crackling forest',
   '- A 07 Swamp Magnetics + B 33 Wormhole Weather — wet membrane / spectral air',
   '- A 11 Cave of Thorns + B 14 Dead Transmission — organ / disintegrating radio',
   '- A 16 Forest Spirit Spiral + B 31 Fossil Frequency — insect / elastic current',
   '- A 21 Hollow Constellation + B 38 Spectral Cicada — pad / microscopic motion',
   '- A 24 Ceremonial Stone + B 46 Ghost Signal Choir — friction / haunted voice',
   '- A 28 Bone Rattle Run + B 43 Gelatinous Orbit — noise bursts / mushroom swarm',
   '', 'All sound sources are procedural and original. No DarkPsy FX466 media used.']
 (banks/'MANIFEST.txt').write_text('NOCTIVORE FX V9: 200 distinct original 48k PCM16 WAV files. No proprietary recordings.\n')
 (root/'SOUND_ATLAS.md').write_text('\n'.join(report)+'\n')
 print('Finished 200 samples',flush=True)
if __name__=='__main__':main()
