# Borato LA-2A User Manual

## Overview

The **Borato LA-2A** is a feedback-topology optical leveling amplifier inspired by the classic Teletronix LA-2A. It combines program-dependent gain reduction, a two-stage release curve, and adjustable analog coloration.

It processes mono or stereo audio signals. In stereo mode, channels are linked by the maximum peak value between both sidechains, preserving stereo imaging during gain reduction.

## Quick Start

1. Switch `Power` **ON** and set mode to `Compress`.
2. Set the VU Meter mode to `GR` (Gain Reduction).
3. Increase `Peak Reduction` until achieving the desired gain reduction.
4. Adjust `Gain` (makeup gain) to match the dry and processed volume.
5. Use `Mix` for parallel compression if needed.
6. Check output peaks and loudness in your DAW. Read the `+4, +10, and GR` section before using the VU meter for absolute level calibration.

As a starting point, aim for **2 to 5 dB** of gain reduction on vocals, bass, or sustained instruments. Response depends heavily on the input signal feeding the optical cell.

## Signal Flow

```text
Input → Input Trim → Input Stage → T4 Attenuation → Gain Stage
      → Tube Stage → Output Transformer → Mix → Output Trim → Output
```

The sidechain employs a feedback topology: it taps the compressed output from the previous sample prior to the `Gain` makeup stage. Thus, `Peak Reduction` controls compression intensity while `Gain` restores level without directly feeding back into the detector.

## Controls Reference

| Control | Range | Default | Function |
|---|---:|---:|---|
| Input | -24 to +24 dB | 0 dB | Input level trim before processing; affects compressor drive. Automatable. |
| Peak Reduction | 0 to 100 | 35 | Sidechain drive. Higher values yield more gain reduction. |
| Gain | -10 to +40 dB | 0 dB | Makeup gain applied after optical attenuation. |
| Meter | +10, +4, or GR | GR | Selects VU needle display mode; does not alter audio processing. |
| Mode | Compress or Limit | Compress | `Limit` applies a steeper curve and permits higher maximum reduction. |
| Power | Off/On | On | In `Off` position, engages hard bypass and parks needle to the left. |
| R37 / HF Sensitivity | 0 to 1 | 0.35 | High-frequency emphasis in the sidechain detector. Interactive screw control. |
| Analog | 0 to 1 | 0.5 | Analog stage saturation/coloration (0 = transparent). Automatable. |
| Mix | 0 to 100% | 100% | Dry/Wet blend between uncompressed input and compressed signal. |
| Output | -24 to +24 dB | 0 dB | Final output level trim after `Mix`. Automatable. |

### Peak Reduction

Mapped quadratically to provide enhanced control resolution in the lower range. Even at `0`, the sidechain receives an attenuated signal rather than being completely disconnected. High-level input signals may generate residual gain reduction, matching the original hardware behavior.

### Compress vs. Limit

- **Compress**: Soft-knee curve with a modeled maximum gain reduction of ~24 dB.
- **Limit**: Steeper knee curve with a modeled maximum gain reduction of ~32 dB.

These represent internal model boundaries rather than fixed compression ratios. Response remains non-linear and program-dependent.

### R37 / HF Sensitivity

The R37 control applies a high-shelf filter to the sidechain without directly EQing the audio signal:

- At `0`, detection is essentially full-band.
- Increasing R37 causes high frequencies to trigger more compression relative to low frequencies.
- At `1`, emphasis reaches approximately +10 dB around 1.8 kHz with level compensation.

Use higher settings to tame sibilance, harsh drum cymbals, or overly bright acoustic guitars.

## Understanding +4, +10, and GR

### Analog Hardware Context

`+4` and `+10` represent output meter calibration reference points, not audio gain stages:

| Position | Analog Hardware Meaning |
|---|---|
| +4 | An output level of +4 dBu registers as 0 VU (+4 dBu studio standard). |
| +10 | An output level of +10 dBu registers as 0 VU (6 dB lower sensitivity than +4). |
| GR | Displays gain reduction in decibels (dB). |

### Current Borato LA-2A Metering Behavior

Switching modes does not alter audio processing. Digital calibration follows the EBU R68 convention (`+4 dBu` ≙ `-18 dBFS`), so 0 VU corresponds to normal studio operating level:

| Position | Internal Calculation | 0 VU Mark |
|---|---:|---:|
| +4 | Output in dBFS + 18 dB | -18 dBFS |
| +10 | Output in dBFS + 12 dB | -12 dBFS |
| GR | Inverted Gain Reduction | 0 dB reduction |

Example: With an output of `-18 dBFS`, `+4` indicates `0 VU` and `+10` indicates `-6`.

### Meter Ballistics

The output meter utilizes an absolute peak envelope with ~200 ms response time and additional visual smoothing. In stereo mode, it displays the higher of the two channels. In `GR` mode, needle ballistics are independently smoothed.

## Program-Dependent Release

The simulated T4 optical cell incorporates:

- ~10 ms attack time
- ~60 ms initial fast recovery
- Adaptive slow tail recovery between 0.5 and 5.0 seconds
- Optical memory effect (slower release after sustained heavy compression)

## Recommended Usage

### Vocals
1. Select `Compress` and set meter to `GR`.
2. Adjust `Peak Reduction` for 2–4 dB reduction on average phrases.
3. Restore level using `Gain`.
4. Optionally increase `R37` slightly if sibilance is present.

### Bass
1. Start in `Compress` mode with low `R37` and `Mix` at 100%.
2. Increase `Peak Reduction` while keeping transient note punch intact.
3. If compression builds up excessively between notes, decrease `Peak Reduction` or input level.

### Parallel Compression
1. Dial in heavier compression than normally required.
2. Dial back `Mix` (e.g., 40–60%) to blend transient punch back in.
3. Adjust `Output` trim to balance overall plugin output.

## Safety & Gain Staging

`Input`, `Gain`, and `Output` can combine to produce substantial output gain. The plugin does not feature an output brickwall limiter. Reduce monitor levels before testing extreme settings.

## Reference

Historical VU metering behavior cross-referenced against the official [Universal Audio LA-2A Manual](https://help.uaudio.com/hc/en-us/articles/19378009641748-LA-2A-Tube-Compressor-Manual). The ranges, defaults, flow, and limitations in this document correspond to the current codebase of the Borato LA-2A.