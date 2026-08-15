# Borato LA-2A

![Borato LA-2A Plugin Interface](screenshot.png)

A **Teletronix LA-2A** optical compressor emulator (VST3 / Standalone) written in **C++20** with **JUCE 8** and **CMake**.

The vintage hardware panel is drawn at runtime with `juce::Graphics` — every knob, screw, switch and the VU meter are native C++ paint code, with no image assets loaded by the plugin.

## Features

- **Optical compression model**: program-dependent gain reduction through a T4-style cell, two-stage release, and a feedback sidechain tapped after the compressor and before makeup gain.
- **Compress / Limit modes**: soft knee up to ~24 dB of modeled reduction, or a steeper knee up to ~32 dB.
- **R37 HF sensitivity**: sidechain high-shelf emphasis (up to ~+10 dB near 1.8 kHz) to tame sibilance without EQing the audio path.
- **Analog stage and Mix**: variable tube/transformer coloration plus a dry/wet blend for parallel compression.
- **Procedurally drawn UI**: brushed metal, patina, reflections, vintage knobs, VU meter and switches, all painted in C++.
- **Cross-platform CMake**: Windows (Visual Studio 2022 / 2026), macOS and Linux, with no hardcoded toolset.

## Requirements

- **CMake** 3.22 or newer
- **JUCE** 8.0.12 or compatible
- **C++20** compliant compiler
- **Windows**: Visual Studio 2022/2026 with **Desktop development with C++** workload

The project is generator-agnostic and does not hardcode an MSVC toolset version. CMake will automatically detect your installed toolset.

## Building on Windows

With JUCE located at `C:\JUCE` and Visual Studio 2026:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DBORATO_JUCE_SOURCE_DIR=C:/JUCE `
  -DBORATO_USE_OPENGL_RENDERER=OFF

cmake --build build --config Release
```

For Visual Studio 2022, simply change the generator:

```powershell
cmake -S . -B build-vs2022 -G "Visual Studio 17 2022" -A x64 `
  -DBORATO_JUCE_SOURCE_DIR=C:/JUCE
```

Do not add `-T v142`, `-T v143`, or `-T v145` flags to the standard workflow. Hardcoding toolsets ties builds to specific local installations and breaks CI workflows.

Build Artifacts:

```text
build/BoratoLA2A_artefacts/Release/Standalone/Borato LA-2A.exe
build/BoratoLA2A_artefacts/Release/VST3/Borato LA-2A.vst3
```

## Locating JUCE

CMake searches for JUCE in the following priority order:

1. `-DBORATO_JUCE_SOURCE_DIR=/path/to/JUCE`
2. Local directory at `C:/JUCE`
3. System `find_package(JUCE)`
4. Automatic download via `-DBORATO_FETCH_JUCE=ON`

The last option is ideal for CI pipelines:

```bash
cmake -S . -B build -DBORATO_FETCH_JUCE=ON
cmake --build build --config Release
```

## Optional OpenGL Renderer

The default JUCE renderer is the primary configuration and operates without dedicated GPU requirements:

```powershell
-DBORATO_USE_OPENGL_RENDERER=OFF
```

To enable and test the OpenGL hardware-accelerated renderer:

```powershell
-DBORATO_USE_OPENGL_RENDERER=ON
```

OpenGL toggles the rendering backend only; the UI layout and procedural caching remain identical.

## Visual Studio Troubleshooting

### `No CMAKE_C_COMPILER could be found`

First check `build/CMakeFiles/CMakeConfigureLog.yaml`. The high-level CMake output can conceal the actual MSBuild diagnostic error.

If your environment contains duplicate environment variables (e.g., both `PATH` and `Path`), MSBuild may fail with:

```text
MSB6001: The item has already been added.
Key in dictionary: 'PATH' Key being added: 'Path'
```

This indicates duplicate environment variables rather than a missing compiler or outdated toolset. Clean environment variables or use a clean PowerShell/Developer Command Prompt to resolve this.

Useful diagnostic commands:

```powershell
cmake --help
cmake --version
Get-ChildItem Env: | Where-Object Name -Match '^(Path|PATH)$'
```

If building after updating or removing Visual Studio versions, clear the build directory:

```powershell
cmake -S . -B build-clean -G "Visual Studio 18 2026" -A x64 `
  -DBORATO_JUCE_SOURCE_DIR=C:/JUCE
```

## Project Structure

```text
Source/
  PluginProcessor.*
  PluginEditor.*
  ui/
    La2aPanelComponent.*
    VuMeterComponent.*
    VintageKnobComponent.*
    ToggleSwitchComponent.*
    JewelLightComponent.*
    RackScrew.*
    GraphicsHelpers.*
```

Internal layout coordinates use a standard `1440 x 1080` canvas. Textures and scratches are procedurally generated outside of the main `paint()` loop for optimal frame rates.

## UI & Compressor Behavior

- **Toggle Switches**: Procedural aged metal rendering with directional highlights, brushed texturing, micro-scratches, and subtle patina.
- **Jewel Light**: Aged nickel bezel with soft lens reflections and realistic glass imperfections.
- **Minimum Compression**: At `Peak Reduction = 0`, hot input signals may still trigger slight gain reduction, modeling the authentic behavior where the sidechain circuit is attenuated but never fully bypassed.

This DSP behavior is validated by regression tests loading the VST3 Release binary and comparing gain reduction across control extremes:

```powershell
.venv\Scripts\python -m pytest tests/test_vu_gr_minimum.py -v -s
```

*Note: Requires `pytest`, `numpy`, and `pedalboard`. Automatically skipped if the VST3 Release binary is not built.*

## Releases

Pushing a `v*` tag runs the multi-OS workflow (Windows x64, macOS arm64/Intel,
Ubuntu) and attaches the VST3 bundles to a draft GitHub Release. Pull requests
run a single Ubuntu build only. The release version is read from the
`project(BoratoLA2A VERSION ...)` line in `CMakeLists.txt`.

## Documentation

For detailed operating instructions, control reference, signal flow, and VU meter alignment details, see the [User Manual](MANUAL.md).

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.
