# scheduler-simulator

Programa que simula o algoritmo de escalonamento de múltiplas filas (com realimentação) com as seguintes características:

1. São utilizadas 3 filas de processos prontos $Q_0$, $Q_1$ e $Q_2$.
2. Os processos iniciam na fila $Q_0$.
3. A fila $Q_0$ usa o escalonamento ***Round Robin*** com quantum de $10ms$.
4. A fila $Q_1 $ usa o escalonamento ***Round Robin*** com quantum de $15ms$.
5. A fila $Q_2 $ usa o escalonamento ***FCFS***.
6. O escalonamento entre as filas é por prioridade com preempção. $Q_0$ é a fila com maior prioridade, $Q_1$ tem prioridade média e $Q_2$ tem prioridade baixa.
7. Um processo passa da fila $Q_0$ para a fila $Q_1$ quando sofre preempção por
tempo.
8. Um processo passa da fila $Q_1$ para a fila $Q_2$ quando sofre preempção por
tempo.
9. Todos os surtos de CPU de um mesmo processo têm a mesma duração.
10. Só existe um dispositivo de E/S no sistema e este só atende um pedido de cada vez.
11. Qualquer operação de E/S leva $20ms$ para ser executada pelo dispositivo.

A entrada consite no número total de processos na fila, a duração do surto de CPU de cada processo e o número de operações de E/S de cada processo:

|**Entrada** | **Saída** |
|:-----------|:---------:|
|2           |           |  
|40 1        |  *Gantt*  |
|15 2        |           | 