# Borato LA-2A

Plugin de áudio VST3/Standalone da Borato Company, construído com C++20, JUCE e CMake. A interface traduz o mock original de um painel vintage para renderização procedural nativa com `juce::Graphics`; HTML, JavaScript, WebView e SVG não são usados em runtime.

## Requisitos

- CMake 3.22 ou mais recente
- JUCE 8.0.12 ou compatível
- Compilador C++20
- Windows: Visual Studio 2022/2026 com o workload **Desktop development with C++**

O projeto é generator-agnostic e não fixa uma versão do MSVC. O CMake deve selecionar o toolset instalado automaticamente.

## Build no Windows

Com JUCE em `C:\JUCE` e Visual Studio 2026:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DBORATO_JUCE_SOURCE_DIR=C:/JUCE `
  -DBORATO_USE_OPENGL_RENDERER=OFF

cmake --build build --config Release
```

Para Visual Studio 2022, troque apenas o generator:

```powershell
cmake -S . -B build-vs2022 -G "Visual Studio 17 2022" -A x64 `
  -DBORATO_JUCE_SOURCE_DIR=C:/JUCE
```

Não adicione `-T v142`, `-T v143` ou `-T v145` ao fluxo normal. Fixar o toolset torna o build dependente de uma instalação específica e prejudica builds futuros e GitHub Actions.

Artefatos Release:

```text
build/BoratoLA2A_artefacts/Release/Standalone/Borato LA-2A.exe
build/BoratoLA2A_artefacts/Release/VST3/Borato LA-2A.vst3
```

## Localização do JUCE

O CMake procura JUCE nesta ordem:

1. `-DBORATO_JUCE_SOURCE_DIR=/caminho/para/JUCE`
2. checkout local em `C:/JUCE`
3. `find_package(JUCE)`
4. download via `-DBORATO_FETCH_JUCE=ON`

O último modo é apropriado para CI:

```bash
cmake -S . -B build -DBORATO_FETCH_JUCE=ON
cmake --build build --config Release
```

## Renderer OpenGL opcional

O renderer padrão do JUCE é a configuração principal e deve funcionar sem GPU dedicada:

```powershell
-DBORATO_USE_OPENGL_RENDERER=OFF
```

Para testar o renderer OpenGL:

```powershell
-DBORATO_USE_OPENGL_RENDERER=ON
```

OpenGL altera somente o backend gráfico; a interface e os caches procedurais permanecem os mesmos.

## Troubleshooting do Visual Studio

### `No CMAKE_C_COMPILER could be found`

Consulte primeiro `build/CMakeFiles/CMakeConfigureLog.yaml`. A mensagem resumida do CMake pode esconder o erro real do MSBuild.

Durante o desenvolvimento deste projeto, o ambiente automatizado continha simultaneamente variáveis chamadas `PATH` e `Path`. O MSBuild 18.7 recusou iniciar `CL.exe` e reportou:

```text
MSB6001: O item já foi adicionado.
Chave contida no dicionário: 'PATH'; chave sendo adicionada: 'Path'
```

Isso não indicava ausência do compilador nem exigia um toolset antigo. O Visual Studio selecionou normalmente o toolset `v145` depois que o processo foi executado com um bloco de ambiente limpo.

Uma atualização do Visual Studio pode ter tornado essa validação mais estrita, mas o problema confirmado é a duplicidade de nomes no ambiente. Em um PowerShell ou Developer Command Prompt normal, o comando padrão acima deve funcionar sem ajustes.

Verificações úteis:

```powershell
cmake --help
cmake --version
Get-ChildItem Env: | Where-Object Name -Match '^(Path|PATH)$'
```

Se o build directory guardar dados de um Visual Studio removido ou atualizado, use um diretório novo:

```powershell
cmake -S . -B build-clean -G "Visual Studio 18 2026" -A x64 `
  -DBORATO_JUCE_SOURCE_DIR=C:/JUCE
```

## Organização

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

As coordenadas internas usam o canvas `1440 x 1080`. Texturas e scratches são determinísticos e pré-calculados fora de `paint()`. Os componentes interativos são separados das camadas decorativas.

## Interface e comportamento do compressor

- Os toggles usam renderização procedural de metal envelhecido, com reflexos direcionais, escovamento, micro-riscos e pátina. O posicionamento e a escala dos switches foram ajustados ao painel.
- A jewel light recebeu aro de níquel envelhecido, reflexos mais suaves e imperfeições na lente para manter a estética vintage.
- Em `Peak Reduction = 0`, sinais de entrada quentes ainda podem produzir redução de ganho. O controle atenua o sidechain ao mínimo, mas não desliga o circuito de compressão.

Esse comportamento do DSP é coberto por um teste de regressão que carrega o VST3 Release e compara a redução de ganho nos extremos do controle:

```powershell
.venv\Scripts\python -m pytest tests/test_vu_gr_minimum.py -v -s
```

O teste requer `pytest`, `numpy` e `pedalboard`, e é ignorado automaticamente quando o VST3 Release ainda não foi compilado.

## Licença

Distribuído sob a licença MIT. Consulte [LICENSE](LICENSE).
