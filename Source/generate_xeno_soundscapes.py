#!/usr/bin/env python3
"""VOIDCORE XENO ATLAS V12: fifty genuinely distinct, deterministic alien soundscapes.

No third-party recordings, random broadband noise, or external sample downloads.
Keeps 50 indexed banks x four MIDI roots at 48 kHz PCM16 for preset compatibility.
"""
from pathlib import Path
import numpy as np
from scipy import signal
from scipy.io import wavfile
import re

SR = 48000
ROOTS = (36, 48, 60, 72)
FAMILIES = (
    ("ABYSSAL GRAVITY", "Sub-octave tectonic drones with slow pressure waves"),
    ("XENO LIQUIDS", "Pitch-diving bubble bursts and wet resonances"),
    ("ALIEN TRANSMISSIONS", "Irregular coded pulses and radio-like formants"),
    ("CREATURE CHOIRS", "Morphing throats and otherworldly vowel clusters"),
    ("CRYSTAL ENTITIES", "Inharmonic glass harmonics and slow metallic swells"),
    ("INSECT HIVE", "Fast microscopic chirps and polyrhythmic swarms"),
    ("PULSAR RITUALS", "Deep gated, hypnotic, gravitational pulses"),
    ("WORMHOLE MOTION", "Sweeping spectral folds and frequency bends"),
    ("NEBULA CHOIRS", "Wide-feeling, slow-moving extraterrestrial pads"),
    ("DIMENSIONAL GLITCH", "Stuttering digital organisms and broken orbital codes"),
)

def bank_info():
    catalog = (Path(__file__).parent / 'BankCatalog.h').read_text(encoding='utf-8')
    rows = re.findall(r'\{"([^"]+)","([^"]+)",(\d+),(true|false)\}', catalog)
    if len(rows) != 50:
        raise ValueError(f'Expected 50 bank slots, got {len(rows)}')
    return rows


def _envelope(t, period, sharp=2.0, phase=0.0):
    """Periodic organic envelopes; no sampling discontinuities."""
    wave = .5 + .5 * np.cos(2 * np.pi * t / period + phase)
    return np.power(wave, sharp)


def create(bank, note, category=0, model=None):
    """Generate an original playable, pitched mono bank at one MIDI root."""
    family = bank % 10
    mutation = bank // 10
    duration = 8.0 + .26 * (bank % 5)
    t = np.arange(round(duration * SR), dtype=np.float64) / SR
    f = 440. * 2. ** ((note - 69) / 12.)
    tau = 2. * np.pi
    phase_offset = ((bank * 97 + note * 43) % 257) * .021
    slow = np.sin(tau * (.035 + .013 * mutation) * t + phase_offset)
    drift = np.sin(tau * (.11 + .019 * (bank % 6)) * t + 1.3 * phase_offset)
    bends = 1. + (.002 + .002 * mutation) * (.7 * slow + .3 * drift)
    phase = tau * f * np.cumsum(bends) / SR
    mod = np.sin(phase * (.39 + mutation * .047) + .8 * slow)
    pulse = _envelope(t, .45 + .16 * mutation, 3.0, phase_offset)
    airy = np.sin(phase * (3.1 + mutation * .37) + 1.9 * slow)

    if family == 0:
        low = np.sin(.5 * phase + 1.8 * slow)
        growl = np.sin(.752 * phase + (1.2 + .6 * mutation) * mod)
        x = .65 * low + .31 * growl + .11 * np.sin(.99 * phase + 2.2 * drift)
        x *= .52 + .48 * _envelope(t, 3.4 + .23 * mutation, 1.1)
    elif family == 1:
        x = np.zeros_like(t)
        count = 8 + 2 * mutation
        for k in range(count):
            start = .35 + k * (duration - .8) / count
            dt = t - start
            active = (dt > 0.) & (dt < .64)
            local = np.maximum(dt, 0.)
            sweep = (1.7 + .21 * mutation + k % 3 * .34) * phase - (12 + k % 5 * 5) * np.exp(-local * 12)
            x += (.43 + .20 * (k % 3 == 0)) * np.sin(sweep + k) * np.exp(-local * (6. + k % 3)) * active
        x += .11 * np.sin(.37 * phase + 1.4 * drift)
    elif family == 2:
        code = np.sin(tau * (1.7 + mutation * .31) * t + 1.2 * drift)
        gate = np.sin(.5 * np.pi * np.clip((code + .45) * 1.10, 0., 1.)) ** 2
        shifting = np.sin(phase * (1.8 + mutation * .11) + 2.4 * mod)
        carrier = np.sin(phase * (.49 + .02 * mutation) + 3.2 * np.sin(.23 * phase + slow))
        x = (shifting * .56 + carrier * .44) * (.06 + .94 * gate)
    elif family == 3:
        morph = .5 + .5 * np.sin(tau * (.09 + mutation * .014) * t + phase_offset)
        u = np.sin(.5 * phase + (2.6 + mutation * .36) * np.sin(.17 * phase + 1.4 * slow))
        a = np.sin(phase * 1.008 + (1.2 + mutation * .24) * np.sin(.81 * phase + drift))
        i = np.sin(phase * 2.13 + 2.0 * np.sin(.48 * phase + slow))
        x = (.65 - .24 * morph) * u + (.20 + .37 * morph) * a + .17 * i
        x *= .78 + .22 * np.sin(tau * .21 * t + slow)
    elif family == 4:
        ratios = (1.0, 1.414 + mutation * .022, 2.31 + mutation * .057, 3.92 + mutation * .13)
        x = sum((.37 / (j + 1) ** .45) * np.sin(phase * ratio + j * 1.13 + (j + 1) * slow)
                for j, ratio in enumerate(ratios))
        x *= .28 + .72 * _envelope(t, 1.7 + .4 * mutation, .75)
    elif family == 5:
        fast = _envelope(t, .12 + .018 * mutation, 8., phase_offset)
        second = _envelope(t, .23 + .027 * mutation, 9., 1.7)
        chirp = np.sin(phase * (2.8 + .43 * mutation) + 8. * mod)
        x = .82 * chirp * (.05 + .65 * fast + .30 * second) + .19 * airy * second
    elif family == 6:
        beat_a = _envelope(t, .67 + mutation * .083, 5.)
        beat_b = _envelope(t, 1.47 + mutation * .11, 7., 1.6)
        sub = np.sin(.5 * phase + 1.8 * slow)
        ring = np.sin(1.47 * phase + 2.7 * mod)
        x = (.77 * sub + .23 * ring) * (.04 + .71 * beat_a + .25 * beat_b)
    elif family == 7:
        sweeping = .55 + .45 * np.sin(tau * (.16 + mutation * .021) * t + slow)
        carrier = np.sin(phase * (.75 + mutation * .079) + (1. + 9 * sweeping) * mod)
        sideband = np.sin(phase * (2.53 + mutation * .17) + 3.7 * np.sin(.29 * phase + drift))
        x = (.73 * carrier + .27 * sideband) * (.64 + .36 * sweeping)
    elif family == 8:
        ratios = (.497, 1.0, 1.004 + mutation * .001, 1.502, 2.003)
        voices = (.23, .36, .28, .19, .13)
        x = sum(w * np.sin(phase * ratio + j * .72 + .4 * j * slow)
                for j, (ratio, w) in enumerate(zip(ratios, voices)))
        x *= .78 + .22 * np.sin(tau * (.055 + .011 * mutation) * t + drift)
    else:
        rate = (3.1 + mutation * 1.07)
        stutter = np.power(.5 + .5 * np.sin(tau * rate * t + 1.8 * slow), 11)
        chopped = np.power(.5 + .5 * np.cos(tau * (rate * .31) * t + phase_offset), 5)
        alien = np.sin(phase * (.63 + mutation * .067) + 3.7 * mod)
        quantized = .65 * np.sin(phase * (1.33 + mutation * .17) + airy) + .35 * np.sin(phase * (.48 + mutation * .09) + drift)
        x = (.6 * alien + .4 * quantized) * (.025 + .68 * stutter + .31 * chopped)

    # Contrasting mutation signatures rather than any shared synthetic base layer.
    if family not in (1, 5, 6, 9):
        x *= .86 + .14 * np.sin(tau * (.12 + .047 * mutation) * t + phase_offset)
    if family in (0, 6):
        cutoff = 2900 + 480 * mutation
    elif family in (4, 5, 7, 9):
        cutoff = 8800 + 420 * mutation
    else:
        cutoff = 5200 + 640 * mutation
    # Four-pole reconstruction lowpass eliminates upper FM and warping
    # sidebands *before* PCM encoding. Note-dependent bandwidth prevents
    # folding at high root notes / on further upward transposition.
    ceiling = min(cutoff, 9200., max(1950., 0.42 * SR - f * 11.))
    sos = signal.butter(4, ceiling, fs=SR, output='sos')
    x = signal.sosfilt(sos, x)
    # Independent spectral shape per bank mutation; no common tonal bed.
    if mutation % 2 == 1:
        hp = signal.butter(2, 65. + 85. * mutation, btype='highpass', fs=SR, output='sos')
        x = signal.sosfilt(hp, x)
    elif mutation == 4:
        shelf = signal.butter(2, 1200. + 330. * family, fs=SR, output='sos')
        x = .60 * x + .40 * signal.sosfilt(shelf, x)
    attack = .08 + .09 * (family in (0, 3, 8))
    x *= -np.expm1(-t / attack)
    x *= -np.expm1(-(duration - t) / .19)
    x -= np.mean(x)
    rms = float(np.sqrt(np.mean(x * x)) + 1e-12)
    target = .125 if family in (0, 3, 8) else .14
    x *= target / rms
    peak = np.max(np.abs(x))
    if peak > .87:
        x *= .87 / peak
    return np.rint(np.clip(x, -1, 1) * 32767).astype('<i2')


def main():
    root = Path(__file__).resolve().parent.parent
    out = root / 'Banks'
    out.mkdir(parents=True, exist_ok=True)
    entries = bank_info()
    for bank, (name, category, kind, one_shot) in enumerate(entries):
        for note in ROOTS:
            wavfile.write(out / f'{bank:02d}_{note}.wav', SR, create(bank, note, int(kind)))
        if bank % 10 == 9:
            print(f'Generated {bank+1}/50 varied xeno banks', flush=True)
    (out / 'MANIFEST.txt').write_text('VOIDCORE XENO V12: 50 distinct 48 kHz banks / 200 original PCM16 WAV files.\n', encoding='utf-8')
    lines = [
        '# VOIDCORE XENO v12 — Diverse Alien Soundscape Atlas', '',
        '50 original multi-zone bank slots / 200 48 kHz PCM16 WAV files.',
        'Each family uses a different sound-generating topology. Every bank also has a distinct mutation.',
        'No third-party recordings or broadband random noise.', '',
        '| Slot | Bank | Synthesis family | Sonic identity |',
        '|---:|---|---|---|',
    ]
    for idx, (name, *_rest) in enumerate(entries):
        family, character = FAMILIES[idx % 10]
        lines.append(f'| {idx:02d} | {name} | {family} | {character} |')
    lines += ['', 'Try strong contrasts: bank A 00 / B 05, A 03 / B 09, A 01 / B 08, A 02 / B 06.',
              'Bank positions and root note filenames are preserved for existing presets.', '']
    (root / 'SOUND_ATLAS.md').write_text('\n'.join(lines), encoding='utf-8')
    print('All 200 XENO soundscapes generated', flush=True)


if __name__ == '__main__':
    main()
