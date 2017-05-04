#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <semaphore.h>

typedef struct {
	pthread_mutex_t mutex_muzi;
	pthread_mutex_t mutex_zeny;
	sem_t emptyRoom;
	int muziCount;
	int zenyCount;
	sem_t muzi;
	sem_t zeny;
	sem_t turnstile;
} Buffer;

typedef struct {
	int id;
	Buffer * buffer;
} WorkerData;

// Inicializacia udajov v buffery
void BufferInit(Buffer * buffer)
{
	pthread_mutex_init(&buffer->mutex_muzi, 0);
	pthread_mutex_init(&buffer->mutex_zeny, 0);
	sem_init(&buffer->emptyRoom, 0, 1);
	sem_init(&buffer->turnstile, 0, 1);
	sem_init(&buffer->zeny, 0, 3);
	sem_init(&buffer->muzi, 0, 3);
	buffer->muziCount = 0;
	buffer->zenyCount = 0;
}

void * muziThread(void *inputData) {
	WorkerData * data = (WorkerData*) inputData;
	Buffer *buffer = data->buffer;
        int ID = data->id;

	while(1) {
		sem_wait(&buffer->turnstile);
		pthread_mutex_lock(&buffer->mutex_muzi);
		buffer->muziCount++;
		if (buffer->muziCount == 1) {
			sem_wait(&buffer->emptyRoom);	
		}
		pthread_mutex_unlock(&buffer->mutex_muzi);
		sem_post(&buffer->turnstile);

		sem_wait(&buffer->muzi);
		printf("kupem sa muz s ID:%2d \n", ID);
		usleep((30 + rand()%4) * 1000);
		printf("odchadzam muz s ID:%2d \n", ID);
		sem_post(&buffer->muzi);

		pthread_mutex_lock(&buffer->mutex_muzi);
		buffer->muziCount--;
		if (buffer->muziCount == 0) {
			sem_post(&buffer->emptyRoom);	
		}
		pthread_mutex_unlock(&buffer->mutex_muzi);
	}
}

void * zenyThread(void *inputData) {
	WorkerData * data = (WorkerData*) inputData;
	Buffer *buffer = data->buffer;
        int ID = data->id;

	while(1) {
		sem_wait(&buffer->turnstile);
		pthread_mutex_lock(&buffer->mutex_zeny);
		buffer->zenyCount++;
		if (buffer->zenyCount == 1) {
			sem_wait(&buffer->emptyRoom);	
		}
		pthread_mutex_unlock(&buffer->mutex_zeny);
		sem_post(&buffer->turnstile);

		sem_wait(&buffer->zeny);
		printf("kupem sa zena s ID:%2d \n", ID);
		usleep((30 + rand()%4) * 1000);
		printf("odchadzam sa zena s ID:%2d \n", ID);
		sem_post(&buffer->zeny);

		pthread_mutex_lock(&buffer->mutex_zeny);
		buffer->zenyCount--;
		if (buffer->zenyCount == 0) {
			sem_post(&buffer->emptyRoom);	
		}
		pthread_mutex_unlock(&buffer->mutex_zeny);
	}
}

// hlavne vlakno
int main(int argc, char *argv[]) {
	int const MUZI = 20;
	int const ZENY = 20;

	pthread_t muziID[MUZI];
	pthread_t zenyID[ZENY];
	int i;
	Buffer buffer;
	WorkerData * data;

	// inicializacia
	srand(time(NULL));
	BufferInit(&buffer);

	// vytvorenie vlakien
	for(i = 0; i < MUZI; i++ ) {
		data = (WorkerData*) malloc(sizeof(WorkerData));
		data->id = i;
		data->buffer = &buffer;
		pthread_create(&muziID[i], NULL, muziThread, (void*) data);
	}

	// vytvorenie vlakien
        for(i = 0; i < ZENY; i++ ) {
                data = (WorkerData*) malloc(sizeof(WorkerData));
                data->id = i;
                data->buffer = &buffer;
                pthread_create(&zenyID[i], NULL, zenyThread, (void*) data);
        }

	// cakanie na ukoncenie vlakien
	for(i = 0; i < MUZI; i++ ) {
		pthread_join(muziID[i], NULL);
	}
	for(i = 0; i < ZENY; i++ ) {
		pthread_join(zenyID[i], NULL);
	}
	printf("vsetky pracovne vlakna ukoncene\n");

	return EXIT_SUCCESS;
}
