#!/usr/bin/env python3
"""Validate the 200 original VOIDCORE V12 soundscapes after generation."""
from pathlib import Path
import numpy as np
from scipy.io import wavfile

ROOT = Path(__file__).resolve().parents[1]
BANKS = ROOT / "Banks"
FILES = sorted(BANKS.glob("*.wav"))
assert len(FILES) == 200, f"Expected 200 WAV files, got {len(FILES)}"
for bank in range(50):
    for root_note in (36, 48, 60, 72):
        assert (BANKS / f"{bank:02d}_{root_note}.wav").is_file(), (bank, root_note)

centroids = []
envelope_cvs = []
for bank in range(10):
    sr, raw = wavfile.read(BANKS / f"{bank:02d}_60.wav")
    assert sr == 48000 and raw.dtype == np.int16 and raw.ndim == 1
    assert 7.0 * sr < len(raw) < 11 * sr, f"Unexpected duration: {bank}"
    y = raw.astype(np.float64) / 32768.0
    assert np.isfinite(y).all()
    rms = float(np.sqrt(np.mean(y * y)))
    assert 0.035 < rms < 0.35, f"Bank {bank} has unusual RMS {rms}"
    assert np.max(np.abs(y)) <= 0.88, f"Bank {bank} clips"
    # Work on a decimated signal for a cheap, deterministic spectral contrast test.
    decimated = y[::16]
    spectrum = np.abs(np.fft.rfft(decimated)) ** 2
    hz = np.fft.rfftfreq(decimated.size, 16 / sr)
    centroids.append(float(np.dot(spectrum, hz) / max(np.sum(spectrum), 1e-15)))
    windows = y[:(len(y) // 4800) * 4800].reshape(-1, 4800)
    loudness = np.mean(np.abs(windows), axis=1)
    envelope_cvs.append(float(loudness.std() / max(loudness.mean(), 1e-9)))

assert max(centroids) - min(centroids) > 270, f"Spectral families too similar: {centroids}"
assert centroids[5] > centroids[0] + 250, "Insect sounds should differ clearly from subvoids"
assert envelope_cvs[6] > 0.40, "Pulsar bank should have recognizable rhythmic dynamics"
print("PASS: 200 WAVs / 50 banks / 10 distinct synthesis families")
print("Representative spectral centroids (Hz):", [round(x) for x in centroids])
print("Envelope movement:", [round(x, 2) for x in envelope_cvs])
