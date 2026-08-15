"""
DSP validation suite for Borato LA-2A (VST3) via pedalboard host emulation.

Covers the measurements from the 2026-08 DSP review:
  I1  static I/O curve (soft knee, emergent ratio, Limit > Compress)
  I2  two-stage / program-dependent release
  I3  sample-rate consistency (44.1 vs 96 kHz)
  I4  low-frequency THD from GR ripple (decides the detector-release fix)
  I5  no clicks when toggling Mode (Compress/Limit) or Power mid-signal
  I6  silence in -> silence out (denormal/DC check with Analog = 1)

Run:  .venv/Scripts/python -m pytest tests/test_dsp_review.py -v -s
"""
import os
import math
import numpy as np
import pytest
from pedalboard import load_plugin

SR = 48000
VST = os.path.abspath(os.path.join(
    os.path.dirname(__file__), "..",
    r"build/BoratoLA2A_artefacts/Release/VST3/Borato LA-2A.vst3/Contents/x86_64-win/Borato LA-2A.vst3"))


def fresh_plugin():
    if not os.path.exists(VST):
        pytest.skip(f"VST3 not built at {VST}")
    return load_plugin(VST)


def neutral(p, pr=50.0, mode="Compress"):
    p.power = True
    p.mode = mode
    p.peak_reduction = pr
    p.input = 0.0
    p.gain = 0.0
    p.output = 0.0
    p.mix = 100.0
    p.analog = 0.0
    p.bypass = False


def tone(freq=1000.0, seconds=3.0, level_db=-6.0, sr=SR):
    n = int(sr * seconds)
    t = np.arange(n) / sr
    x = (np.sin(2 * np.pi * freq * t) * (10 ** (level_db / 20.0))).astype(np.float32)
    return np.vstack([x, x])


def rms_db(audio, skip=0.7):
    n = audio.shape[-1]
    tail = audio[..., int(n * skip):]
    r = math.sqrt(float(np.mean(tail.astype(np.float64) ** 2)) + 1e-20)
    return 20 * math.log10(r + 1e-12)


def window_rms_db(audio, start_s, dur_s, sr=SR):
    a = int(start_s * sr)
    b = a + int(dur_s * sr)
    seg = audio[..., a:b].astype(np.float64)
    return 20 * math.log10(math.sqrt(float(np.mean(seg ** 2))) + 1e-12)


# ---------------------------------------------------------------- I1
def test_static_io_curve():
    levels = list(range(-40, 1, 4))
    curves = {}
    for mode in ("Compress", "Limit"):
        grs = []
        for lvl in levels:
            p = fresh_plugin()
            neutral(p, pr=50.0, mode=mode)
            x = tone(level_db=lvl)
            gr = rms_db(x) - rms_db(p(x, SR))
            grs.append(gr)
        curves[mode] = grs
        print(f"\n{mode}: " + "  ".join(
            f"{l}dB->GR{g:+.1f}" for l, g in zip(levels, grs)))

    for mode, grs in curves.items():
        # Soft knee / emergent ratio: GR grows with level, no steps backwards
        for a, b in zip(grs, grs[1:]):
            assert b >= a - 0.3, f"{mode}: GR not monotonic with input level"
        # And grows gradually (soft knee), not as a hard-threshold jump
        deltas = [b - a for a, b in zip(grs, grs[1:])]
        assert max(deltas) < 4.0, f"{mode}: knee too hard for an LA-2A ({max(deltas):.1f} dB per 4 dB step)"

    assert curves["Limit"][-1] > curves["Compress"][-1] + 1.0, \
        "Limit mode should produce more GR than Compress at hot levels"


# ---------------------------------------------------------------- I2
def _release_gr_trace(burst_seconds):
    """Loud burst then quiet tail; returns GR (dB) at offsets after the drop."""
    p = fresh_plugin()
    neutral(p, pr=75.0)
    tail_s = 4.0
    x = np.concatenate([
        tone(seconds=burst_seconds, level_db=-6.0),
        tone(seconds=tail_s, level_db=-30.0)], axis=1)
    y = p(x, SR)
    drop = burst_seconds
    win = 0.03
    offsets = (0.05, 0.3, 3.5)
    return {
        off: window_rms_db(x, drop + off, win) - window_rms_db(y, drop + off, win)
        for off in offsets
    }


def test_two_stage_program_dependent_release():
    short = _release_gr_trace(2.0)
    long = _release_gr_trace(8.0)
    print(f"\nGR after drop (2 s burst): {short}")
    print(f"GR after drop (8 s burst): {long}")

    # Recovery exists and is not instantaneous: still recovering at +300 ms
    assert short[0.05] > short[3.5] + 1.0, "no recovery after signal drops"
    assert short[0.3] > short[3.5] + 0.3, "release fully done by 300 ms — no slow tail"
    # Program dependence: longer sustained compression -> slower recovery
    assert long[0.3] >= short[0.3] - 0.1, "8 s burst should not release faster than 2 s burst"


# ---------------------------------------------------------------- I3
def test_sample_rate_consistency():
    grs = {}
    for sr in (44100, 96000):
        p = fresh_plugin()
        neutral(p, pr=60.0)
        x = tone(sr=sr)
        grs[sr] = rms_db(x) - rms_db(p(x, sr))
    print(f"\nsteady-state GR: {grs}")
    assert abs(grs[44100] - grs[96000]) < 0.5, \
        "GR should not depend on sample rate"


# ---------------------------------------------------------------- I4
def _thd(y, f0, sr):
    n = y.shape[-1]
    seg = y[0, n // 2:].astype(np.float64)
    w = np.hanning(seg.size)
    spec = np.abs(np.fft.rfft(seg * w))
    freqs = np.fft.rfftfreq(seg.size, 1.0 / sr)

    def band(f):
        idx = int(np.argmin(np.abs(freqs - f)))
        return float(spec[max(0, idx - 3):idx + 4].max())

    fund = band(f0)
    harm = math.sqrt(sum(band(f0 * k) ** 2 for k in range(2, 11)))
    return harm / max(fund, 1e-12)


def test_low_frequency_thd():
    p = fresh_plugin()
    neutral(p, pr=75.0)
    x = tone(freq=50.0, seconds=4.0, level_db=-6.0)
    y = p(x, SR)
    gr = rms_db(x) - rms_db(y)
    thd = _thd(y, 50.0, SR)
    print(f"\n50 Hz under {gr:.1f} dB GR: THD = {thd * 100:.3f}%")
    # Review criterion: >1% THD from GR ripple triggers the asymmetric
    # detector-release fix (C2). Keep this as the acceptance threshold.
    assert thd < 0.01, "GR ripple is audibly distorting low frequencies"


# ---------------------------------------------------------------- I5
def _max_step(seg):
    return float(np.abs(np.diff(seg.astype(np.float64), axis=-1)).max())


def _click_check(param_name, from_value, to_value):
    """Process a steady tone in chunks, flip one parameter mid-stream, and
    compare the max sample-to-sample step around the flip against steady state."""
    p = fresh_plugin()
    neutral(p, pr=60.0)
    setattr(p, param_name, from_value)

    chunk = tone(seconds=1.0, level_db=-6.0)
    y_pre = p.process(chunk, SR, reset=False)
    y_pre = p.process(chunk, SR, reset=False)  # settled
    setattr(p, param_name, to_value)
    y_post = p.process(chunk, SR, reset=False)

    # A click is a step larger than the steady-state slope of EITHER state
    # (the destination state may be legitimately louder, e.g. bypass).
    steady = max(_max_step(y_pre[0]), _max_step(y_post[0, -int(0.5 * SR):]))
    flip = _max_step(y_post[0, : int(0.2 * SR)])
    print(f"\n{param_name} {from_value}->{to_value}: steady step {steady:.4f}, "
          f"post-toggle step {flip:.4f}")
    assert flip < steady * 1.5 + 1e-4, \
        f"toggling {param_name} produces a click (step {flip:.4f} vs steady {steady:.4f})"


def test_mode_toggle_no_click():
    _click_check("mode", "Compress", "Limit")


def test_power_toggle_no_click():
    _click_check("power", True, False)


# ---------------------------------------------------------------- I6
def test_silence_in_silence_out():
    p = fresh_plugin()
    neutral(p, pr=75.0)
    p.analog = 1.0
    # Warm the state with signal first, then feed silence
    p.process(tone(seconds=2.0, level_db=-6.0), SR, reset=False)
    y = p.process(np.zeros((2, SR * 5), dtype=np.float32), SR, reset=False)
    peak_tail = float(np.abs(y[:, -SR:]).max())
    print(f"\npeak of last second of silence: {peak_tail:.2e}")
    assert peak_tail < 1e-6, "silence should decay to (numerical) zero"
