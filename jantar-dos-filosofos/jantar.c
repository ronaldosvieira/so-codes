#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int idt;
    int *left_fork, *right_fork;
} thread_arg, *ptr_thread_arg;

void* philosophers(void *arg) {
    ptr_thread_arg t_arg = (ptr_thread_arg) arg;

    printf("Oi, sou o filosofo %d! Meus garfos sao o %d e o %d!\n", 
        t_arg->idt, *t_arg->left_fork, *t_arg->right_fork);
}

int main (int argc, char **argv) {
    int i;
    int num_philosophers;

    // checa argumento
    if (argv[1] != NULL) num_philosophers = atoi(argv[1]);
    else {
        printf("Modo de usar: %s (qtd. filosofos > 1)\n", argv[0]);
        return EXIT_FAILURE;
    }

    // inicializa garfos
    int forks[num_philosophers];
    for (i = 0; i < num_philosophers; i++) forks[i] = i;

    printf("Num. de filosofos: %d\n", num_philosophers);

    pthread_t threads[num_philosophers];
    thread_arg thread_args[num_philosophers];

    for (i = 0; i < num_philosophers; i++) {
        // adiciona id da thread aos argumentos
        thread_args[i].idt = i;

        // adiciona garfos da esquerda e direita aos argumentos
        thread_args[i].left_fork = &forks[i];
        thread_args[i].right_fork = &forks[(i + 1) % num_philosophers];

        // cria a thread i
        pthread_create(&(threads[i]), NULL, philosophers, &(thread_args[i]));
    }

    for (i = 0; i < num_philosophers; i++) {
        pthread_join(threads[i], NULL);
    }

    return EXIT_SUCCESS;
}
