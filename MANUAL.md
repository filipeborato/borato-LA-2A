# Manual do Borato LA-2A

## Visão geral

O Borato LA-2A é um compressor óptico de topologia feedback inspirado no LA-2A. Combina redução de ganho dependente do programa, release em dois estágios e coloração analógica ajustável.

Aceita áudio mono ou estéreo. Em estéreo, os canais são vinculados pelo maior valor instantâneo entre os sidechains, preservando a imagem estéreo durante a redução de ganho.

## Início rápido

1. Ligue `Power` e selecione `Compress`.
2. Coloque o medidor em `GR`.
3. Aumente `Peak Reduction` até obter a redução desejada.
4. Ajuste `Gain` para comparar o volume processado com o original.
5. Use `Mix` para compressão paralela, se necessário.
6. Confira picos e loudness na DAW. Leia a seção `+4, +10 e GR` antes de usar o VU para calibração absoluta.

Como ponto de partida, procure 2 a 5 dB de redução em vocais, baixo ou instrumentos sustentados. A resposta depende do sinal que alimenta a célula óptica.

## Fluxo de sinal

```text
Entrada → Input → estágio de entrada → atenuação T4 → Gain
        → válvula → transformador de saída → Mix → Output → Saída
```

O sidechain é feedback: observa a saída comprimida do sample anterior, antes de `Gain`. Assim, `Peak Reduction` controla a compressão e `Gain` repõe o nível sem realimentar diretamente o detector.

## Controles

| Controle | Faixa | Padrão | Função |
|---|---:|---:|---|
| Input | -24 a +24 dB | 0 dB | Nível antes do circuito; também altera quanto o compressor reage. Disponível ao host. |
| Peak Reduction | 0 a 100 | 35 | Drive do sidechain. Valores maiores geram mais redução. Não é ganho de entrada. |
| Gain | -10 a +40 dB | 0 dB | Compensação depois da atenuação óptica. |
| Meter | +10, +4 ou GR | GR | Escolhe somente a leitura da agulha; não altera o áudio. |
| Mode | Compress ou Limit | Compress | `Limit` usa curva mais íngreme e permite mais redução. |
| Power | Off/On | On | Em `Off`, aplica bypass direto e estaciona a agulha à esquerda. |
| R37 / HF Sensitivity | 0 a 1 | 0,35 | Ênfase de agudos apenas no sidechain. O parafuso `R37` é interativo. |
| Analog | 0 a 1 | 0,5 | Coloração dos estágios analógicos; 0 é transparente. Disponível ao host. |
| Mix | 0 a 100% | 100% | Mistura o caminho comprimido com o sinal depois de `Input`. |
| Output | -24 a +24 dB | 0 dB | Trim final depois de `Mix`. Disponível ao host. |

### Peak Reduction

O mapeamento é quadrático para dar mais resolução na parte baixa. Mesmo em `0`, o sidechain recebe sinal atenuado; ele não é desligado. Material muito quente pode produzir compressão residual de alguns decibéis. Esse comportamento é coberto por teste de regressão.

### Compress e Limit

- `Compress`: knee suave e redução máxima modelada de 24 dB.
- `Limit`: curva mais íngreme e redução máxima modelada de 32 dB.

Esses são limites internos do modelo, não ratios fixos. A resposta é não linear e dependente do programa.

### R37 / HF Sensitivity

O R37 aplica um high-shelf ao sidechain, sem equalizar diretamente o áudio:

- Em `0`, a detecção fica praticamente full-band.
- Ao aumentar, agudos acionam mais compressão e graves acionam relativamente menos.
- Em `1`, a ênfase chega a aproximadamente 10 dB em torno de 1,8 kHz, com compensação de nível.

Use valores maiores para controlar sibilância, pratos ou guitarras brilhantes.

## Entendendo +4, +10 e GR

### Significado no equipamento analógico

`+4` e `+10` são calibrações do medidor de saída, não ganho adicional:

| Posição | Significado analógico |
|---|---|
| +4 | Uma saída de +4 dBu deve indicar 0 VU. |
| +10 | Uma saída de +10 dBu deve indicar 0 VU. Para o mesmo sinal, a agulha fica 6 dB abaixo de +4. |
| GR | Mostra quantos decibéis o compressor reduz. |

`dBu` mede tensão analógica; `dBFS` mede nível digital relativo ao teto. Não existe conversão universal sem escolher um nível de alinhamento.

### Comportamento atual do Borato LA-2A

O deslocamento relativo de 6 dB está correto e o seletor não entra no processamento de áudio. A calibração absoluta digital, porém, ainda não representa uma referência analógica utilizável:

| Posição | Cálculo atual | 0 VU atual |
|---|---:|---:|
| +4 | saída em dBFS | 0 dBFS |
| +10 | saída em dBFS - 6 dB | +6 dBFS |
| GR | redução com sinal invertido para a escala | 0 dB de redução |

Exemplo: com saída de `-12 dBFS`, `+4` aponta para `-12` e `+10` para `-18`. A troca muda apenas a agulha.

Na prática, use `GR` para ajustar a compressão e os medidores da DAW para nível absoluto. `+4` e `+10` servem hoje para comparação relativa, não para alinhamento confiável de 0 VU.

Uma calibração digital convencional poderia definir `0 VU = -18 dBFS` em `+4`; assim, 0 VU em `+10` corresponderia a `-12 dBFS`. Esse alinhamento não está implementado.

### Balística

A saída usa envelope de pico absoluto com suavização de aproximadamente 200 ms e suavização visual adicional. É um indicador lento, mas não implementa integralmente um VU analógico RMS. Em estéreo, mostra o maior canal.

Em `GR`, a leitura é suavizada separadamente. Sete decibéis de redução levam a agulha à marca `-7`.

## Release dependente do programa

A célula T4 simulada combina:

- ataque de aproximadamente 10 ms;
- recuperação rápida de aproximadamente 60 ms;
- cauda lenta adaptativa entre 0,5 e 5 segundos;
- memória que torna o release mais lento após compressão sustentada.

A agulha e o áudio podem demorar a voltar ao repouso depois de trechos densos. Isso faz parte do modelo.

## Procedimentos recomendados

### Vocal

1. Selecione `Compress` e `GR`.
2. Ajuste `Peak Reduction` para frases comuns reduzirem 2 a 4 dB.
3. Reponha o volume com `Gain`.
4. Se necessário, aumente moderadamente o R37 para sibilância.
5. Compare em loudness semelhante.

### Baixo

1. Comece em `Compress`, R37 baixo e `Mix` em 100%.
2. Aumente `Peak Reduction` sem eliminar o ataque.
3. Se o release acumular entre notas, reduza `Peak Reduction` ou a entrada.

### Compressão paralela

1. Ajuste compressão mais forte que o necessário.
2. Reduza `Mix` para reinserir o ataque.
3. Ajuste `Output` para manter o volume comparável.

## Ganho e segurança

`Input`, `Gain` e `Output` podem somar bastante ganho. O plugin não possui limiter final. Reduza a monitoração antes de experimentar valores extremos e confira o medidor de pico da DAW.

Os principais controles de ganho, mix, analog e sidechain têm suavização de cerca de 30 ms. Mudanças de modo e R37 são aplicadas por bloco.

## Referência

A função histórica do medidor foi conferida no [manual oficial do LA-2A da Universal Audio](https://help.uaudio.com/hc/en-us/articles/19378009641748-LA-2A-Tube-Compressor-Manual). As faixas, padrões, fluxo e limitações deste documento correspondem ao código atual do Borato LA-2A.