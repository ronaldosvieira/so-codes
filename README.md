# Trabalhos de Sistemas Operacionais
Trabalhos realizados na disciplina de Sistemas Operacionais do curso de Ciência da Computação da UFRRJ em 2016.1.

## Terminal
Um terminal simples em C utilizando as funções ```fork``` e ```exec```.
* Diretório: ```terminal/```
* Arquivos: ```terminal.c```

## Multiplicação de matrizes
Algoritmo de multiplicação de matrizes quadradas paralelo. Parâmetros: ```./mult-matrizes-xxxx.o <arquivo de entrada> <opcional: numero de processos/threads>```
* Diretório: ```mult-matrizes/```
* Arquivos: ```mult-matrizes-processos.c```: utilizando processos e compartilhamento de memória através das funções ```shmget``` e ```shmat```. Compilar com ```gcc (...) -lm```.