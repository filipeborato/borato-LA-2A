# Borato LA-2A v0.0.2

DSP polish release: click-free mode/power switching, correct startup gain
staging and a properly calibrated VU meter — driven by a full DSP review with
a new measurement-based test suite.

![Borato LA-2A](https://raw.githubusercontent.com/filipeborato/borato-LA-2A/master/screenshot.png)

> All artifacts are unsigned development builds.

## Fixed

- **Click when toggling Compress / Limit** — the two gain-reduction curves are
  now interpolated with a 50 ms crossfade instead of switching instantly
  (previously up to an ~11 dB GR step in a single sample).
- **Click when toggling Power** — hard bypass replaced by a 20 ms crossfade to
  the raw input; the GR meter decays to zero with it.
- **Level burst at transport start** — parameter smoothers now snap to the
  actual knob values in `prepareToPlay` instead of ramping from defaults for
  the first 30 ms (up to 24 dB too loud with negative Output trim).
- **Host reset support** — `AudioProcessor::reset()` now clears the T4 cell
  state, so the up-to-8 s program-dependent release tail no longer survives a
  transport reset or loop jump.
- **VU meter `+4` / `+10` calibration** — now referenced to EBU R68
  (`+4 dBu` ≙ `-18 dBFS`): 0 VU = -18 dBFS in `+4` mode, -12 dBFS in `+10`
  mode. Previously 0 VU sat at digital full scale, pinning the needle to the
  left at normal operating levels. See MANUAL.md.

## Validated (new `tests/test_dsp_review.py`, pedalboard host emulation)

- Static I/O curve: soft knee, emergent ratio, Limit > Compress at hot levels.
- Two-stage program-dependent release (fast recovery + slow tail).
- Sample-rate consistency: identical GR at 44.1 and 96 kHz (< 0.001 dB).
- Low-frequency THD from GR ripple: 0.52% at 50 Hz under 13 dB GR.
- No clicks on Mode/Power toggles; silence in → silence out (no denormals/DC).

No changes to the compression calibration itself — Peak Reduction behaviour,
T4 time constants and the Analog stage are identical to v0.0.1.

## Downloads

| Platform | Asset |
|---|---|
| Windows x64 | `BoratoLA2A-v0.0.2-Windows-x64-VST3.zip` |
| macOS Apple Silicon | `BoratoLA2A-v0.0.2-macOS-arm64-VST3.zip` |
| macOS Intel | `BoratoLA2A-v0.0.2-macOS-intel-VST3.zip` |
| Linux (Ubuntu) | `BoratoLA2A-v0.0.2-Ubuntu-VST3.tar.gz` |

## Install

- **Windows** — unzip and copy `Borato LA-2A.vst3` to
  `C:\Program Files\Common Files\VST3\`
- **macOS** — unzip and copy `Borato LA-2A.vst3` to
  `~/Library/Audio/Plug-Ins/VST3/`. If Gatekeeper blocks it:
  `xattr -dr com.apple.quarantine "Borato LA-2A.vst3"`
- **Linux** — extract and copy `Borato LA-2A.vst3` to `~/.vst3/`

Then rescan plugins in your DAW.

## Known limitations

- VST3 only in this release (the Standalone build exists but is not shipped as
  an asset; AU and CLAP are not built yet).
- At `Peak Reduction = 0` the sidechain is attenuated, not disconnected, so hot
  inputs still produce residual gain reduction. This matches the hardware.
