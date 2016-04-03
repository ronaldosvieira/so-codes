#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/sysinfo.h>
#include <pthread.h>

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

typedef struct {
    int idt;
    int start_col, end_col;
    int width, height;
    float ***A_ptr, ***B_ptr, ***C_ptr;
} thread_arg, *ptr_thread_arg;

void* thread_func(void *arg) {
    ptr_thread_arg t_arg = (ptr_thread_arg) arg;
    int j, i, k, steps = 0;
    float aux;

    // faz a multiplicacao entre as matrizes e guarda em C
    for (i = 0; i < t_arg->height; i++) {
        for (j = t_arg->start_col; j < t_arg->end_col; j++) {
            aux = 0.0f;

            for (k = 0; k < t_arg->width; k++) {
                aux += (*t_arg->A_ptr)[i][k] * (*t_arg->B_ptr)[k][j];
                steps++;
            }

            (*t_arg->C_ptr)[i][j] = aux;
        }
    }

    // cada thread printa seu td e qtd de passos
    printf("Passos na thread %d: %d\n", t_arg->idt, steps);
}

int main (int argc, char **argv) {
    float **A, **B, **C;
    int iA, jA, iB, jB, iC, jC, i;
    int height, width;
    int num_threads;

    FILE *input_file;

    // checa argumentos
    if (argv[1] == NULL || (argv[2] != NULL && atoi(argv[2]) < 1)) {
        printf("Modo de usar: %s (arquivo) (?qtd. processos > 0)\n", argv[0]);
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
    if (argv[2] != NULL) num_threads = atoi(argv[2]);
    else {
        if (width <= get_nprocs()) num_threads = width;
        else num_threads = get_nprocs();
    }

    printf("Num. de threads: %d\n", num_threads);

    A = (float**) malloc(height * sizeof(float*));
    B = (float**) malloc(height * sizeof(float*));
    C = (float**) malloc(height * sizeof(float*));

    for (i = 0; i < height; i++) {
        A[i] = (float*) malloc(width * sizeof(float));
        B[i] = (float*) malloc(width * sizeof(float));
        C[i] = (float*) malloc(width * sizeof(float));
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

    printf("\nArquivo %s lido!\n\n", argv[1]);

    // fecha o arquivo
    fclose(input_file);

    pthread_t threads[num_threads];
    thread_arg thread_args[num_threads];

    for (i = 0; i < num_threads; i++) {
        thread_args[i].idt = i;

        thread_args[i].width = width;
        thread_args[i].height = height;

        // cada thread obtem ponteiros para as matrizes
        thread_args[i].A_ptr = &A;
        thread_args[i].B_ptr = &B;
        thread_args[i].C_ptr = &C;

        // cada thread obtem as colunas que tera que calcular
        // colunas = [start_col, end_col[
        thread_args[i].start_col = ((int) (1.0f * width / num_threads) * i);
        thread_args[i].end_col = ((int) (1.0f * width / num_threads) * (i + 1));

        // cria a thread i
        pthread_create(&(threads[i]), NULL, thread_func, &(thread_args[i]));
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // exibe a matriz resultante
    printf("\nResultado:");
    printMatrix(C, width, height);

    // desaloca as matrizes
    free(A);
    free(B);
    free(C);

    return EXIT_SUCCESS;
}
