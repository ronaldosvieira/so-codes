#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <math.h>

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
    int shm_fd, num_procs;
    int pid, pnum;
    int start_col, end_col, steps;

    const char* mem_name = "mult_matriz";

    if (argv[1] == NULL || argv[2] == NULL || (argv[3] != NULL && atoi(argv[3]) < 1)) {
        printf("Modo de usar: matriz_CPU.o (linhas) (colunas) (?qtd. processos > 0)\n");
        return 1;
    }

    printf("Processadores disponiveis: %d\n", get_nprocs());

    int width = atoi(argv[1]), height = atoi(argv[2]);

    // caso o num. de processos nao for especificado
    // pega o menor entre o num. de colunas e o num. de processadores disponiveis
    if (argv[3] != NULL) num_procs = atoi(argv[3]);
    else {
        if (width <= get_nprocs()) num_procs = width;
        else num_procs = get_nprocs();
    }

    printf("Num. de processos: %d\n", num_procs);

    shm_fd = shm_open(mem_name, O_CREAT | O_TRUNC | O_RDWR, 0666);

    // aloca as matrizes
    A = mmap(0, height * sizeof(float*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
    B = mmap(0, height * sizeof(float*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
    C = mmap(0, height * sizeof(float*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);

    for (jC = 0; jC < height; jC++) {
        A[jC] = mmap(0, width * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
        B[jC] = mmap(0, width * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
        C[jC] = mmap(0, width * sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
    }
    
    // inicializa as matrizes A e B
    // TODO: subst. por leitura de arquivo?
    for (jC = 0; jC < height; jC++) {
        for (iC = 0; iC < width; iC++) {
            int kC = jC * width + iC;
            A[jC][iC] = (float) kC + 1;
            if (jC == iC)
                B[jC][iC] = 1.0f;
            else
                B[jC][iC] = 0.0f;
        }
    }

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
    start_col = floor((1.0f * width / num_procs) * pnum);
    end_col = floor((1.0f * width / num_procs) * (pnum + 1));


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


    // "desaloca" as matrizes
    munmap(A, sizeof A);
    munmap(B, sizeof B);
    munmap(C, sizeof C);

    shm_unlink(mem_name);

    return EXIT_SUCCESS;
}