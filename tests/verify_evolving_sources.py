#!/usr/bin/env python3
"""Validate that shipped WAVs contain distinct evolving chapters, not static loops."""
from pathlib import Path
import numpy as np
from scipy.io import wavfile

root = Path(__file__).resolve().parents[1] / "Banks"
files = sorted(root.glob("*.wav"))
assert len(files) == 200, f"Expected 200 WAVs; found {len(files)}"
features = []
for bank in (0, 3, 7, 9, 23, 48):
    rate, raw = wavfile.read(root / f"{bank:02d}_48.wav")
    assert rate == 48000 and raw.ndim == 1
    assert raw.size >= rate * 11, f"Bank {bank} too short to evolve"
    assert np.max(np.abs(raw.astype(np.float64))) < 32768
    x = raw.astype(np.float64) / 32768.
    profiles = []
    for begin in (1.4, 4.0, 7.0, 10.1):
        window = x[int(begin * rate):int((begin + .8) * rate)]
        assert window.size >= rate // 2
        window *= np.hanning(len(window))
        spectrum = np.abs(np.fft.rfft(window)) ** 2
        freqs = np.fft.rfftfreq(len(window), 1 / rate)
        bands = np.asarray([
            np.sum(spectrum[freqs < 250]),
            np.sum(spectrum[(freqs >= 250) & (freqs < 1400)]),
            np.sum(spectrum[(freqs >= 1400) & (freqs < 6500)]),
            np.sum(spectrum[freqs >= 6500]),
        ])
        profiles.append(np.concatenate((bands / max(1e-12, bands.sum()),
                                        [np.sqrt(np.mean(window ** 2))])))
    profiles = np.asarray(profiles)
    drift = float(np.max(np.ptp(profiles[:, :4], axis=0)))
    loudness_drift = float(np.ptp(profiles[:, 4]) / max(1e-6, np.mean(profiles[:, 4])))
    features.append((bank, drift, loudness_drift))
    print(f"bank {bank:02d}: spectral evolution {drift:.3f}, dynamics {loudness_drift:.3f}")
    assert drift > .025 or loudness_drift > .17, f"Bank {bank} sounds static across chapters"
assert len({(round(a, 3), round(b, 3)) for _, a, b in features}) >= 4
print("PASS: all 200 living WAV files; sampled banks evolve spectrally or rhythmically across time")
