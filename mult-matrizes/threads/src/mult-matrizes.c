#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/sysinfo.h>
#include <pthread.h>

#define DEBUG 0
#define printflush(s, ...) do {if (DEBUG) {printf(s, ##__VA_ARGS__); fflush(stdout);}} while (0)

#define MAX_NUM 5

typedef struct {
    int idt;
    int start_col, end_col;
    int width, height;
    int ***A_ptr, ***B_ptr, ***C_ptr;
} thread_arg, *ptr_thread_arg;

int** initMatrix(int m, int n) {
    int** mat = (int**) malloc(sizeof(int*) * m);
    
    for (int i = 0; i < m; i++) {
        mat[i] = (int*) malloc(sizeof(int) * n);
    }
    
    return mat;
}

int** generateRandomMatrix(int** mat, int m, int n) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            mat[i][j] = rand() % MAX_NUM;
        }
    }
    
    return mat;
}

void printMatrix(int** mat, int m, int n) {
    printflush("[");
    
    for (int i = 0; i < m; i++) {
        printflush(" ");
        
        for (int j = 0; j < n; j++) {
            printflush("%d ", mat[i][j]);
        }
        
        if (i != m - 1) printflush("\n");
    }
    
    printflush("]\n");
}

void* thread_func(void *arg) {
    ptr_thread_arg t_arg = (ptr_thread_arg) arg;
    int j, i, k, steps = 0;
    int aux;

    // faz a multiplicacao entre as matrizes e guarda em C
    for (i = 0; i < t_arg->height; i++) {
        for (j = t_arg->start_col; j < t_arg->end_col; j++) {
            aux = 0;

            for (k = 0; k < t_arg->width; k++) {
                aux += (*t_arg->A_ptr)[i][k] * (*t_arg->B_ptr)[k][j];
                steps++;
            }

            (*t_arg->C_ptr)[i][j] = aux;
        }
    }

    // cada thread printa seu td e qtd de passos
    printflush("Passos na thread %d: %d\n", t_arg->idt, steps);
}

int main (int argc, char **argv) {
    int **A, **B, **C;
    int iA, jA, iB, jB, iC, jC, i;
    int height, width;
    int num_threads;

    FILE *input_file;

    // checa argumentos
    if (argv[1] == NULL || atoi(argv[1]) < 1 || 
        (argv[2] != NULL && atoi(argv[2]) < 1)) {
        printflush("Modo de usar: %s (tamanho das matrizes > 0) (?qtd. threads > 0)\n", argv[0]);
        return EXIT_FAILURE;
    }

    width = height = atoi(argv[1]);

    printflush("Processadores disponiveis: %d\n", get_nprocs());

    // caso o num. de threads nao for especificado
    // pega o menor entre o tam. da matriz e o num. de processadores disponiveis
    if (argv[2] != NULL) num_threads = atoi(argv[2]);
    else {
        if (width <= get_nprocs()) num_threads = width;
        else num_threads = get_nprocs();
    }

    printflush("Num. de threads: %d\n", num_threads);

    A = (int**) initMatrix(width, height);
    B = (int**) initMatrix(width, height);
    C = (int**) initMatrix(width, height);

    generateRandomMatrix(A, width, height);
    generateRandomMatrix(B, width, height);

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
        thread_args[i].start_col = (int) ((1.0f * width / num_threads) * i);
        thread_args[i].end_col = (int) ((1.0f * width / num_threads) * (i + 1));

        // cria a thread i
        pthread_create(&(threads[i]), NULL, thread_func, &(thread_args[i]));
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // exibe a matriz resultante
    printflush("\nResultado:");
    printMatrix(C, width, height);

    // desaloca as matrizes
    free(A);
    free(B);
    free(C);

    return EXIT_SUCCESS;
}
