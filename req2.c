#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#define NUMBER_OF_JOBS 1000

sem_t sync, delay_consumer;

int syncValue, delay_consumer_value;
int sharedValue = 0;
int elementsProduced = 0;

void *producer(void *p);
void *consumer(void *p);
void visualisation();

void *producer(void *p)
	{
	while(elementsProduced != NUMBER_OF_JOBS) 
		{
		sem_getvalue(&sync, &syncValue);
		if(syncValue == 1)
			{
			sem_wait(&sync);
			sharedValue++;
			sem_post(&sync);
			visualisation();
			elementsProduced++;
			if(elementsProduced == NUMBER_OF_JOBS)
				{
				sem_post(&delay_consumer);
				}
			}
		}
	sem_post(&delay_consumer);
	}

void *consumer(void *p)
	{
	sem_wait(&delay_consumer);	
	while(sharedValue != 0)
		{
		sem_getvalue(&sync, &syncValue);
		if (syncValue == 1)
			{
			sem_wait(&sync);
			sharedValue--;
			sem_post(&sync);
			visualisation();
			if(sharedValue == 0)
				{
				sem_wait(&delay_consumer);
				}
			}
		}
	}

void visualisation()
	{
	printf("iIndex = %d\n", sharedValue);
	}

int main()
	{
	sem_init(&sync, 0, 1);
	sem_init(&delay_consumer, 0, 0); 

	pthread_t prodThread, conThread;

	pthread_create(&conThread, NULL, consumer, NULL);
	pthread_create(&prodThread, NULL, producer, NULL);

	pthread_join(conThread, NULL);
	pthread_join(prodThread, NULL);

	sem_getvalue(&sync, &syncValue);
	sem_getvalue(&delay_consumer, &delay_consumer_value);

	printf("sSync = %d, sDelayConsumer = %d\n", syncValue, delay_consumer_value);	
 
	return 0;
	}
