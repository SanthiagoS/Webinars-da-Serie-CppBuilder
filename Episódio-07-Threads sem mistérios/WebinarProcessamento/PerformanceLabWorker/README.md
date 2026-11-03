# PerformanceLabWorker

Aplicacao VCL didatica para comparar o mesmo laboratorio do
PerformanceLabBaseline com uma arquitetura baseada em Worker Thread.
O projeto usa Win64x como target padrao e nao depende do executavel
Baseline.

## Arquitetura

- A Main Thread recebe cliques, executa timers de responsividade e atualiza
  componentes VCL.
- A Worker Thread executa a simulacao, o mesmo calculo de
  `sin`/`sqrt`/`cos` e a persistencia CSV por item.
- Resultados sao enviados para a Main Thread por `TThread::Queue`.
- Nenhuma rotina da Worker acessa labels, barras, grid ou charts.
- O cancelamento e cooperativo; o formulario sinaliza cancelamento, aguarda
  a thread e invalida callbacks antes de destruir a janela.

## Comparacao

Mantenha carga, duracao, persistencia e configuracao iguais nos dois projetos.
O loop de stress usa 1.024 iteracoes por lote e os mesmos calculos do
baseline. A diferenca didatica e o local da execucao:

- Baseline: calculo e persistencia bloqueiam a Main Thread.
- Worker: calculo e persistencia ficam na Worker Thread; somente a pintura do
  grid e a atualizacao agrupada dos graficos retornam para a Main Thread.

O botao Testar resposta continua disponivel durante o stress. Observe os
cliques atendidos, o maior atraso, o pulso da UI e o uso separado de CPU do
sistema, processo, Main Thread e Worker Thread.

## Compilacao

Abra `PerformanceLabWorker.cbproj`, selecione Win64x e compile com Ctrl+F9.
Para Release, use:

```bat
msbuild PerformanceLabWorker.cbproj /t:Build /p:Config=Release /p:Platform=Win64x
```

Os CSVs sao gravados em Documentos/PerformanceLabWorker. Cada item abre o
arquivo, grava, solicita `FlushFileBuffers` e fecha, como no baseline.

