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

void think(int i) {
    printf("Filósofo %d está pensando...\n", i);
}

void eat(int i) {
    printf("Filósofo %d está comendo. :9\n", i);
}

void* philosophers(void *arg) {
    ptr_thread_arg t_arg = (ptr_thread_arg) arg;

    while(1) {
        think(t_arg->idt);

        if ((*t_arg->left_fork) || (*t_arg->right_fork)) {
            printf("Treta!\n");
        }

        (*t_arg->left_fork) = 1;
        (*t_arg->right_fork) = 1;

        eat(t_arg->idt);

        if (!(*t_arg->left_fork) || !(*t_arg->right_fork)) {
            printf("Treta!\n");
        }

        (*t_arg->left_fork) = 0;
        (*t_arg->right_fork) = 0;
    }
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
    for (i = 0; i < num_philosophers; i++) forks[i] = 0;

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

    for (i = 0; i < num_philosophers; i++) {
        printf("forks[%d] = %d\n", i, forks[i]);
    }

    return EXIT_SUCCESS;
}
