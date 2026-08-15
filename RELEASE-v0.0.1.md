# Borato LA-2A v0.0.1

First public release of the **Borato LA-2A**, a feedback-topology optical
leveling amplifier inspired by the classic Teletronix LA-2A, built with C++20,
JUCE 8 and CMake.

![Borato LA-2A](https://raw.githubusercontent.com/filipeborato/borato-LA-2A/master/screenshot.png)

> All artifacts are unsigned development builds.

## Highlights

- 🎚️ **Optical compression model** — program-dependent T4 cell attenuation with a
  two-stage release curve; the sidechain taps the compressed output (feedback
  topology), so `Peak Reduction` sets the compression and `Gain` only restores
  level.
- 🎛️ **Compress / Limit** — soft-knee up to ~24 dB of modeled gain reduction, or
  a steeper knee up to ~32 dB.
- 🔧 **R37 HF sensitivity** — sidechain high-shelf emphasis (up to ~+10 dB around
  1.8 kHz) to tame sibilance and harsh cymbals without EQing the audio path.
- 🌡️ **Analog stage** — continuously variable tube/transformer coloration, from
  transparent to saturated.
- 🎚️ **Mix and trims** — dry/wet for parallel compression, plus input and output
  trims (±24 dB).
- 🖥️ **100% procedural panel** — brushed metal, patina, VU meter, vintage knobs
  and switches drawn in real time with `juce::Graphics`; no WebView, HTML, JS or
  runtime SVG.
- 🔊 **Stereo linking** — channels share the maximum sidechain peak, preserving
  the stereo image during gain reduction.

## Parameters

| Control | Range | Default |
|---|---:|---:|
| Input | -24 to +24 dB | 0 dB |
| Peak Reduction | 0 to 100 | 35 |
| Gain | -10 to +40 dB | 0 dB |
| Meter | +10 / +4 / GR | GR |
| Mode | Compress / Limit | Compress |
| Power | Off / On | On |
| R37 (HF sensitivity) | 0 to 1 | 0.35 |
| Analog | 0 to 1 | 0.5 |
| Mix | 0 to 100% | 100% |
| Output | -24 to +24 dB | 0 dB |

Full control reference and metering notes in [MANUAL.md](MANUAL.md).

## Downloads

| Platform | Asset |
|---|---|
| Windows x64 | `BoratoLA2A-v0.0.1-Windows-x64-VST3.zip` |
| macOS Apple Silicon | `BoratoLA2A-v0.0.1-macOS-arm64-VST3.zip` |
| macOS Intel | `BoratoLA2A-v0.0.1-macOS-intel-VST3.zip` |
| Linux (Ubuntu) | `BoratoLA2A-v0.0.1-Ubuntu-VST3.tar.gz` |

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
- VU meter `+4` / `+10` keep the correct 6 dB offset between them, but absolute
  calibration is referenced to digital full scale — see MANUAL.md.
- At `Peak Reduction = 0` the sidechain is attenuated, not disconnected, so hot
  inputs still produce residual gain reduction. This matches the hardware.
