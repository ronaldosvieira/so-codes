#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

typedef int semaphore;

typedef struct {
    int idt;
    int num_cars;
    int slots;
} thread_arg, *ptr_thread_arg;

sem_t car_sem, plot_sem, mutex;
int waiting = 0;
int done = 0;
int qt = 0;

void let_parking() {
	//printf("Um carro está estacionando...\n");
	//fflush(stdin);
}

void park(int time_amount) {
	//printf("Carro %d estacionou por %d usecs...\n", i, i);
	//fflush(stdin);
	++qt;
	usleep(time_amount);
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

	if (waiting < t_arg->slots) {
		waiting++;

		int time_amount = rand() % 10;

		printf("Carro %d estacionou por %d usecs...\n", t_arg->idt, time_amount);
		
		sem_post(&car_sem);
		sem_post(&mutex);
		sem_wait(&plot_sem);

		park(time_amount);
	} else {
		sem_post(&mutex);
	}

	printf("Carro %d foi embora!!!\n", t_arg->idt);
}

int main(int argc, char **argv) {
	srand(time(NULL));

	int i, slots, num_cars;

	if (argv[1] == NULL || argv[2] == NULL || atoi(argv[1]) < 0 || atoi(argv[2]) < 0) {
        printf("Modo de usar: %s (vagas >= 0) (qtd carros >= 0)\n", argv[0]);
        return EXIT_FAILURE;
    }

    slots = atoi(argv[1]);
    num_cars = atoi(argv[2]);

	pthread_t parking_lot;
	pthread_t cars[num_cars];
	thread_arg car_args[num_cars];

	sem_init(&car_sem, 0, 5);
	sem_init(&plot_sem, 0, 0);
	sem_init(&mutex, 0, 1);

	pthread_create(&parking_lot, NULL, parking_lot_func, NULL);

	for (i = 0; i < num_cars; i++) {
        car_args[i].idt = i;
        car_args[i].slots = slots;
        car_args[i].num_cars = num_cars;

        pthread_create(&(cars[i]), NULL, cars_func, &(car_args[i]));
    }

    for (i = 0; i < num_cars; i++) {
    	pthread_join(cars[i], NULL);
    }

    done = 1;

	sem_destroy(&car_sem);
	sem_destroy(&plot_sem);
	sem_destroy(&mutex);

	printf("qtd de carros estacionados = %d\n", qt);
}