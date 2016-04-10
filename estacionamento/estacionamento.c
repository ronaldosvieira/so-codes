#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define CHAIRS 5

typedef int semaphore;

typedef struct {
    int idt;
} thread_arg, *ptr_thread_arg;

sem_t cust_sem, barb_sem, mutex;
int waiting = 0;
int done = 0;

void cut_hair() {
	//printf("Cortando cabelo...\n");
	//fflush(stdin);
}

void get_haircut(int i) {
	printf("Cliente %d recebendo corte...\n", i);
	fflush(stdin);
}

void *barber_func(void *arg) {
	while (!done) {
		sem_wait(&cust_sem);
		sem_wait(&mutex);
		
		waiting--;
		
		sem_post(&barb_sem);
		sem_post(&mutex);

		cut_hair();
	}
}

void *customer_func(void *arg) {
	ptr_thread_arg t_arg = (ptr_thread_arg) arg;

	printf("Customer %d ativado!!!\n", t_arg->idt);

	sem_wait(&mutex);

	if (waiting < CHAIRS) {
		waiting++;
		sem_post(&cust_sem);
		sem_post(&mutex);
		sem_wait(&barb_sem);

		get_haircut(t_arg->idt);
	} else {
		sem_post(&mutex);
	}
}

int main() {
	int i;
	pthread_t barber;
	pthread_t customers[CHAIRS];
	thread_arg cust_args[CHAIRS];

	sem_init(&cust_sem, 0, 0);
	sem_init(&barb_sem, 0, 0);
	sem_init(&mutex, 0, 1);

	pthread_create(&barber, NULL, barber_func, NULL);

	for (i = 0; i < CHAIRS; i++) {
        cust_args[i].idt = i;

        pthread_create(&(customers[i]), NULL, customer_func, &(cust_args[i]));
    }

    for (i = 0; i < CHAIRS; i++) {
    	pthread_join(customers[i], NULL);
    }

    done = 1;

	sem_destroy(&cust_sem);
	sem_destroy(&barb_sem);
	sem_destroy(&mutex);
}