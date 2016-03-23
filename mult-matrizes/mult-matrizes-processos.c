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
    int height, width;
    int shm_fd, num_procs;
    int pid, pnum;
    int start_col, end_col, steps;

    FILE *input_file;

    const char* mem_name = "mult_matriz";

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