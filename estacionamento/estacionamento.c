#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define SLOTS 10
#define NUM_CARS 100

typedef int semaphore;

typedef struct {
    int idt;
} thread_arg, *ptr_thread_arg;

sem_t car_sem, plot_sem, mutex;
int waiting = 0;
int done = 0;
int qtd = 0;

void let_parking() {
	//printf("Cortando cabelo...\n");
	//fflush(stdin);
}

void park(int i) {
	//printf("Carro %d estacionou por %d msecs...\n", i, i);
	//fflush(stdin);
	qtd++;
}

void *parking_lot_func(void *arg) {
	while (!done) {
		sem_wait(&car_sem);
		sem_wait(&mutex);
		
		waiting--;
		
		sem_post(&plot_sem);
		sem_post(&mutex);

		let_parking();
	}
}

void *cars_func(void *arg) {
	ptr_thread_arg t_arg = (ptr_thread_arg) arg;

	sem_wait(&mutex);

	printf("Carro %d chegou!!!\n", t_arg->idt);

	if (waiting < SLOTS) {
		waiting++;

		printf("Carro %d estacionou por %d msecs...\n", t_arg->idt, t_arg->idt);
		
		sem_post(&car_sem);
		sem_post(&mutex);
		sem_wait(&plot_sem);

		park(t_arg->idt);
	} else {
		sem_post(&mutex);
	}

	printf("Carro %d foi embora!!!\n", t_arg->idt);
}

int main() {
	int i;
	pthread_t parking_lot;
	pthread_t cars[NUM_CARS];
	thread_arg car_args[NUM_CARS];

	sem_init(&car_sem, 0, 5);
	sem_init(&plot_sem, 0, 0);
	sem_init(&mutex, 0, 1);

	pthread_create(&parking_lot, NULL, parking_lot_func, NULL);

	for (i = 0; i < NUM_CARS; i++) {
        car_args[i].idt = i;

        pthread_create(&(cars[i]), NULL, cars_func, &(car_args[i]));
    }

    for (i = 0; i < NUM_CARS; i++) {
    	pthread_join(cars[i], NULL);
    }

    done = 1;

	sem_destroy(&car_sem);
	sem_destroy(&plot_sem);
	sem_destroy(&mutex);

	printf("qtd = %d\n", qtd);
}