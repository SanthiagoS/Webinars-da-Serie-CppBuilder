# PerformanceLabBaseline

Aplicacao VCL didatica para RAD Studio / C++Builder 13 Florence. O projeto usa
Windows 64-bit Modern (Win64x) como target padrao, com Debug e Release.
Todos os controles visuais estao declarados em MainForm.dfm e publicados em
TfrmPerformanceLabBaseline. Nao depende de componentes de terceiros.

## Abrir e executar

Abra PerformanceLabBaseline.cbproj, selecione Win64x e compile com Ctrl+F9.
Execute com F9. Para comparacoes de performance, use Release em ambas as
versoes e mantenha maquina, carga, persistencia e tamanho da janela iguais.
O executavel fica em Win64x/Debug ou Win64x/Release.
BuildRelease.cmd tambem compila Release quando a variavel BDS aponta para
a instalacao do RAD Studio.

Na linha de comando do RAD Studio:

```bat
msbuild PerformanceLabBaseline.cbproj /t:Build /p:Config=Release /p:Platform=Win64x
```

## Roteiro para o webinar

1. Comece na carga 1, com aumento automatico e CSV habilitados.
2. Observe o pulso da UI, o tempo por ciclo e o maior tempo entre as etapas.
3. A carga sobe um nivel a cada oito ciclos concluidos, ate 20. Tambem pode
   ser alterada pelo controle deslizante durante a execucao.
4. Mova a janela, use a barra de rolagem ou clique em Testar resposta.
   Em carga alta, os comandos aguardam a Main Thread terminar o ciclo.
5. Pare, desmarque CSV e execute novamente com a mesma carga para comparar
   o custo da persistencia com os calculos. Iniciar zera as metricas e
   preserva a carga escolhida; Limpar volta a carga para 1.
6. Para comparacao manual, desative o aumento automatico e use cargas 1,
   5, 10 e 20. O historico guarda os ultimos 150 ciclos.

## Stress CPU controlado e latencia do botao

Selecione 3, 4 ou 5 segundos (padrao: 4) e clique em Disparar stress CPU.
O disparo para o pipeline baseline e executa calculos de seno, cosseno e
raiz na Main Thread ate atingir o prazo medido por relogio monotonicamente
crescente. O resultado e exibido para manter o trabalho observavel em
Release. Nao ha Sleep, ProcessMessages ou bombeamento de mensagens no
trecho critico. O prazo e verificado a cada 1.024 iteracoes; a duracao e
aproximada e pode exceder o alvo sob preempcao ou pausa no depurador.

Clique e solte Testar resposta durante o bloqueio. Quando ele terminar,
a UI volta ao processamento normal de mensagens e exibe a ultima e a
maior latencia. A medicao parte de MSG.time da mensagem WM_LBUTTONUP
(ou WM_KEYUP da barra de espaco com o botao focado), e termina no OnClick.
Assim, inclui a espera na fila Windows, nao apenas o tempo do handler.
Nao inclui o tempo entre pressionar e soltar o botao. Usa a diferenca
unsigned de GetTickCount, tolerando a virada do contador de 32 bits.
Cliques programaticos sem mensagem de entrada nao alteram a latencia.

Os valores permanecem entre disparos de stress e sao zerados por Limpar
ou Iniciar. O pulso da UI continua apos o stress; Parar o desativa.
O pipeline baseline nao reinicia automaticamente: Iniciar abre uma nova
sessao. O resumo de stress mostra duracao, tempo de CPU do processo,
numero de iteracoes e resultado. O stress isolado nao grava CSV nem
substitui as metricas das quatro etapas do ultimo ciclo baseline.
Todos os controles novos e TApplicationEvents estao no DFM.

## O que custa tempo

O evento do TTimer executa sequencialmente:

- Simulacao: gera 25 vezes o nivel de carga em leituras deterministicas.
- Calculos: para cada sensor, executa 2.000 vezes o nivel de carga em
  iteracoes de seno, cosseno e raiz. O resultado aparece no grid e no CSV;
  o trabalho nao e um loop vazio descartavel pelo otimizador.
- Atualizacao visual: altera cada linha do TStringGrid e chama Repaint
  por sensor, propositalmente sem agrupar atualizacoes.
- Persistencia: abre, posiciona, grava, solicita FlushFileBuffers e fecha
  o arquivo para cada sensor, na mesma thread da interface.

A quantidade de sensores cresce linearmente e o trabalho total de calculo
cresce quadraticamente com a carga. Nao existe Sleep para simular custo,
Application.ProcessMessages, processamento paralelo ou servico externo.
O timer de pulso tambem executa na Main Thread.

## Interpretacao das metricas

- Ciclo: soma dos tempos reais das quatro etapas, medidos por relogio
  monotonicamente crescente. Exclui o custo posterior de atualizar as
  metricas e acrescentar o resumo ao historico.
- Tempo total: tempo de parede desde Iniciar, atualizado por ciclo.
- CPU: tempo de CPU do processo dividido pelo intervalo de parede;
  mostra tanto a porcentagem da maquina quanto o equivalente de um nucleo.
  Uma Main Thread saturada pode representar uma fracao pequena da CPU
  total em maquinas com muitos processadores logicos.
- Atraso UI: excesso do intervalo observado entre pulsos sobre os 100 ms
  programados, com maximo da sessao. Inclui variacao do agendador Windows.
  Durante o bloqueio os numeros ficam congelados; o atraso aparece quando
  a UI volta a atender o timer.
- Excesso: tempo das etapas acima do orcamento nominal de 100 ms.
- Barras: participacao percentual de cada etapa no ciclo. Maior etapa
  identifica o gargalo medido; vermelho indica ciclo acima do orcamento.

Timers VCL nao garantem periodicidade exata nem acumulam uma fila de todos
os ticks perdidos. O atraso exibido nao representa uma fila de amostras.
Parar e fechar a janela sao atendidos depois que o ciclo em andamento
termina. Isso faz parte da demonstracao da sobrecarga da Main Thread.

## Dados e limites

Cada execucao com persistencia cria um CSV exclusivo em
Documentos/PerformanceLabBaseline. O caminho aparece na barra de status.
As colunas sao cycle;sensor;value;result, com separador ponto e virgula e
numeros formatados conforme a localidade do Windows. O arquivo cresce
durante a sessao; Limpar nao apaga os CSVs. Falhas de gravacao interrompem
o processamento e aparecem na barra de status; um ciclo pode ficar
parcialmente persistido.

A memoria do grid acompanha no maximo 500 sensores, e o historico visual
e limitado. O CSV continua crescendo ate a parada. FlushFileBuffers mede
a solicitacao sincrona ao Windows; o custo observado depende do disco,
cache, antivirus e ambiente. O maior gargalo nao e fixado artificialmente.

Arquivos principais: PerformanceLabBaseline.cpp (entrada),
MainForm.h (formulario e estado), MainForm.cpp (pipeline e metricas),
MainForm.dfm (componentes VCL) e PerformanceLabBaseline.cbproj (projeto).

## Validacao realizada

Revisao do stress em 06/10/2026: Win64x Debug compilado pela IDE sem
erros/avisos; Release por MSBuild sem erros (aviso ambiental MSB4056
na etapa de PCH). Validacao funcional em executavel temporario, com o
mesmo formulario/DFM e calculo otimizado em Release: trechos criticos
de 3.000,0 / 5.000,0 / 4.000,0 ms e CPU de 2.984,4 / 5.000,0 / 4.000,0 ms.
Mensagens de mouse postadas durante o bloqueio foram atendidas somente
depois, com latencias de 2.656 / 4.672 / 3.703 ms; o maior valor permaneceu
em 4.672 ms. Espaco no botao focado tambem validado (2.656 ms).
Verificados retorno do pulso, Limpar, clique programatico sem medicao,
reinicio do pipeline sem CSV e Parar. Sao medicoes locais, com janela
fora da tela e entrada enfileirada por automacao.

Compilacao Win64x Debug na IDE sem erros ou avisos; Release por MSBuild
com sucesso (o ambiente emitiu o aviso MSB4056 sobre STA/MTA).
Teste funcional de abertura, inicio, parada e CSV: carga 1 em aproximadamente
33 ms por ciclo e carga 20 entre 1,1 e 1,3 s, com 3.500 registros na sessao
de carga alta. Sao observacoes locais de teste automatizado com janela
oculta, nao resultados de benchmark de renderizacao para o webinar.
