#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <sys/shm.h>

void printMatrix(float **m, float w, float h) {
    int i, j;
    
    printf("\n");
    
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            printf("%.2f ", m[j][i]);
        }
        printf("\n");
    }
}

int main (int argc, char **argv) {
    float **A, **B, **C;
    int iA, jA, iB, jB, iC, jC, i;
    int height, width;
    int shm_A_ptr, shm_B_ptr, shm_C_ptr;
    int num_procs;
    int pid, pnum;
    int start_col, end_col, steps;

    FILE *input_file;

    // checa argumentos
    if (argv[1] == NULL || (argv[2] != NULL && atoi(argv[2]) < 1)) {
        printf("Modo de usar: matriz_CPU.o (arquivo) (?qtd. processos > 0)\n");
        return EXIT_FAILURE;
    }

    // abre o arquivo (sai em caso de erro)
    input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        printf("Erro: problema ao abrir o arquivo.\n");
        return EXIT_FAILURE;
    }

    // le width e height do arquivo
    fscanf(input_file, "%d %d", &width, &height);

    printf("Processadores disponiveis: %d\n", get_nprocs());

    // caso o num. de processos nao for especificado
    // pega o menor entre o num. de colunas e o num. de processadores disponiveis
    if (argv[2] != NULL) num_procs = atoi(argv[2]);
    else {
        if (width <= get_nprocs()) num_procs = width;
        else num_procs = get_nprocs();
    }

    printf("Num. de processos: %d\n", num_procs);

    int shm_A[height], shm_B[height], shm_C[height];

    // obtem memoria compartilhada para a primeira dimensao da matriz A
    if ((shm_A_ptr = shmget(IPC_PRIVATE, height * sizeof(float*), IPC_CREAT | 0666)) < 0) {
        perror("erro no shmget");
        exit(1);
    }

    // obtem memoria compartilhada para a primeira dimensao da matriz B
    if ((shm_B_ptr = shmget(IPC_PRIVATE, height * sizeof(float*), IPC_CREAT | 0666)) < 0) {
        perror("erro no shmget");
        exit(1);
    }

    // obtem memoria compartilhada para a primeira dimensao da matriz C
    if ((shm_C_ptr = shmget(IPC_PRIVATE, height * sizeof(float*), IPC_CREAT | 0666)) < 0) {
        perror("erro no shmget");
        exit(1);
    }

    A = (float**) shmat(shm_A_ptr, NULL, 0);
    B = (float**) shmat(shm_B_ptr, NULL, 0);
    C = (float**) shmat(shm_C_ptr, NULL, 0);

    // obtem memoria compartilhada para a segunda dimensao
    // das matrizes A, B e C
    for (i = 0; i < height; i++) {
        if ((shm_A[i] = shmget(IPC_PRIVATE, width * sizeof(float), IPC_CREAT | 0666)) < 0) {
            perror("shmget i");
            exit(1);
        }

        if ((shm_B[i] = shmget(IPC_PRIVATE, width * sizeof(float), IPC_CREAT | 0666)) < 0) {
            perror("shmget i");
            exit(1);
        }

        if ((shm_C[i] = shmget(IPC_PRIVATE, width * sizeof(float), IPC_CREAT | 0666)) < 0) {
            perror("shmget i");
            exit(1);
        }

        A[i] = (float*) shmat(shm_A[i], NULL, 0);
        B[i] = (float*) shmat(shm_B[i], NULL, 0);
        C[i] = (float*) shmat(shm_C[i], NULL, 0);
    }

    // le as matrizes A e B do arquivo
    for (jC = 0; jC < height; jC++) {
        for (iC = 0; iC < width; iC++) {
            fscanf(input_file, "%f", &A[jC][iC]);
        }
    }

    fscanf(input_file, "\n");

    for (jC = 0; jC < height; jC++) {
        for (iC = 0; iC < width; iC++) {
            fscanf(input_file, "%f", &B[jC][iC]);
        }
    }

    printf("\nArquivo %s lido!\n", argv[1]);

    // fecha o arquivo
    fclose(input_file);

    // pnum do processo pai = 0
    pnum = 0;

    // "forka" num_procs - 1 processos
    for (i = 1; i < num_procs; i++) {
        pid = fork();

        if (pid) break;

        pnum = i;
    }

    steps = 0;

    // cada processo obtem as colunas que tera que calcular
    // colunas = [start_col, end_col[
    start_col = ((int) (1.0f * width / num_procs) * pnum);
    end_col = ((int) (1.0f * width / num_procs) * (pnum + 1));


    // faz a multiplicacao entre as matrizes e guarda em C
    for (jC = 0; jC < height; jC++) {
        for (jC = 0; jC < height; jC++) {
            for (iC = start_col; iC < end_col; iC++) {
                float aux = 0.0f;

                for (iA = 0; iA < width; iA++) {
                    aux += A[jC][iA] * B[iA][iC];
                    steps++;
                }

                C[jC][iC] = aux;
            }
        }
    }

    if (!pnum) printf("\n");

    // cada processo printa seu pnum e qtd de passos
    printf("Passos no proc. %d: %d\n", pnum, steps);

    wait(NULL);

    // sai de todos os processos filhos
    if (pnum) return 0;

    // exibe a matriz resultante
    printf("\nResultado:");
    printMatrix(C, width, height);

    // "desaloca" as matrizes e devolve a memoria compartilhada
    shmdt(A);
    shmdt(B);
    shmdt(C);

    shmctl(shm_A_ptr, IPC_RMID, 0);
    shmctl(shm_B_ptr, IPC_RMID, 0);
    shmctl(shm_C_ptr, IPC_RMID, 0);

    return EXIT_SUCCESS;
}
