"""
Host-emulation test for Borato LA-2A (VST3) using pedalboard.

Sets parameters DIRECTLY on the plugin (bypassing the GUI/APVTS attachments),
so it isolates the DSP: if the compressor still reduces gain with
Peak Reduction at minimum, that is a DSP fact, not a meter/GUI display bug.

Context: the user was confused because the GR meter shows negative dB
(deflection = active gain reduction) even with Peak Reduction turned all the
way down. This test measures actual output-level reduction at
peak_reduction=0 across a range of signal levels to confirm whether real
compression is happening, and at what level it kicks in.

Run:  .venv/Scripts/python -m pytest tests/test_vu_gr_minimum.py -v -s
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


def tone(freq=1000.0, seconds=3.0, level_db=-6.0):
    n = int(SR * seconds)
    t = np.arange(n) / SR
    x = (np.sin(2 * np.pi * freq * t) * (10 ** (level_db / 20.0))).astype(np.float32)
    return np.vstack([x, x])


def rms_db(audio, skip=0.7):
    """RMS in dBFS of the steady-state tail (skip first `skip` fraction so the
    T4 cell's slow release/memory has settled)."""
    n = audio.shape[-1]
    tail = audio[..., int(n * skip):]
    r = math.sqrt(float(np.mean(tail.astype(np.float64) ** 2)) + 1e-20)
    return 20 * math.log10(r + 1e-12)


def neutral_min_peak_reduction(p):
    """Isolate the compressor's own GR: no makeup/output trim, no mix
    dilution, no analog coloration, Peak Reduction at its documented
    minimum ("PR = 0 (sem compressão)" per OptoCompressor.h)."""
    p.power = True
    p.mode = "Compress"
    p.peak_reduction = 0.0
    p.input = 0.0
    p.gain = 0.0
    p.output = 0.0
    p.mix = 100.0
    p.analog = 0.0
    p.bypass = False


def test_minimum_peak_reduction_sweep():
    """Sweep signal level with Peak Reduction pinned at 0 and report GR.

    If the DSP genuinely produced ~0 dB of gain reduction at PR=0 (matching
    the "sem compressão" comment), output level should track input level
    within a fraction of a dB at every level tested. Any level at which
    output is measurably quieter than input indicates the T4 cell is being
    driven into compression even at the knob's minimum setting.
    """
    print(f"\nVST: {VST}")
    levels = [-30.0, -24.0, -18.0, -12.0, -9.0, -6.0, -3.0, 0.0]
    results = []
    for lvl in levels:
        p = fresh_plugin()
        neutral_min_peak_reduction(p)
        x = tone(level_db=lvl)
        y = p(x, SR)
        din, dout = rms_db(x), rms_db(y)
        gr = din - dout
        results.append((lvl, din, dout, gr))
        print(f"  in(RMS)={din:+6.2f} dBFS -> out(RMS)={dout:+6.2f} dBFS   GR={gr:+.2f} dB")

    worst_gr = max(gr for _, _, _, gr in results)
    print(f"\nWorst-case GR at Peak Reduction = 0: {worst_gr:+.2f} dB")

    # This assertion documents current behaviour rather than presupposing
    # "correct": with sidechainDriveMinDb = -18 dB (OptoCompressor.h) and
    # elThresholdLin = 0.02 (~-34 dBFS), any tone louder than ~-16 dBFS
    # should already show measurable GR. Fails loudly if that's NOT what
    # happens, so we know the earlier code-reading analysis was wrong.
    assert worst_gr > 1.0, (
        "Expected measurable GR at some signal level even with Peak Reduction "
        "at minimum (per the -18 dB drive floor analysis) -- if this fails, "
        "the DSP is NOT reproducing the reported behaviour and the meter "
        "wiring should be re-examined instead."
    )


def test_minimum_vs_maximum_peak_reduction():
    """Sanity check: PR=0 should still show *less* GR than PR=100 for the
    same signal, proving the knob has effect even though PR=0 isn't silent."""
    x = tone(level_db=-6.0)

    p_min = fresh_plugin()
    neutral_min_peak_reduction(p_min)
    gr_min = rms_db(x) - rms_db(p_min(x, SR))

    p_max = fresh_plugin()
    neutral_min_peak_reduction(p_max)
    p_max.peak_reduction = 100.0
    gr_max = rms_db(x) - rms_db(p_max(x, SR))

    print(f"\nGR at PR=0:   {gr_min:+.2f} dB")
    print(f"GR at PR=100: {gr_max:+.2f} dB")
    assert gr_max > gr_min + 3.0, "Peak Reduction knob should meaningfully increase GR"
