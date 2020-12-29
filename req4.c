#include <stdio.h> 
#include <stdlib.h>
#include <semaphore.h> 
#include <sys/types.h>
#include <pthread.h> 

#define NUMBER_OF_JOBS 1000
#define BOUNDED_BUFFER_SIZE 100
#define NUMBER_OF_CONSUMERS 4

sem_t sync, empty, full; 

int syncValue, emptyValue, fullValue; 

int elementsProduced = 0;
int elementsConsumed = 0;

char boundedBuffer[BOUNDED_BUFFER_SIZE];

void *producer(void *p); 
void *consumer(void *p); 
void visualisation(); 
void semaphoreValues();

void *producer(void *p)
	{
	while(elementsProduced != NUMBER_OF_JOBS)
		{
		sem_getvalue(&sync, &syncValue);
		if(syncValue == 1)
			{
			sem_wait(&sync);

			int j;
	 		for(j=0; j<BOUNDED_BUFFER_SIZE; j++)
				{
				if(boundedBuffer[j] == 0)
					{
					boundedBuffer[j] = 42;
					elementsProduced++;
					sem_wait(&empty);
					break;
					}
				}
			
			sem_post(&sync);

			visualisation();
			//printf("number of elements produced is %d\n", elementsProduced);			
			//semaphoreValues();			

			//if there is at least one element in the buffer, wake up consumer
			//if empty value is between 1 and BOUNDED_BUFFER_SIZE, set full from 0 to 1 (enable consumer)
			
			sem_getvalue(&empty, &emptyValue);
			sem_getvalue(&full, &fullValue);

			if((emptyValue > 0 && emptyValue <= BOUNDED_BUFFER_SIZE) && fullValue == 0)
				{
				sem_post(&full);
				}

			if(emptyValue == 0 && fullValue != 1)
				{
				sem_post(&full);		
				} 
			}
		} 
	}

void *consumer(void *p)
	{
	while(elementsConsumed != NUMBER_OF_JOBS)
		{
		sem_getvalue(&sync, &syncValue);
		if(syncValue == 1)
			{
			sem_wait(&sync);
			
			if(boundedBuffer[0] == 42)
				{
				boundedBuffer[0] = 0;
				elementsConsumed++;
				sem_post(&empty);
				int j;
				for(j=0; j<BOUNDED_BUFFER_SIZE; j++)
					{
					int tmp = boundedBuffer[j+1];
					boundedBuffer[j] = tmp;			
					}
				}

			sem_post(&sync); 	

			//printf("elements consumed is %d\n", elementsConsumed);
			visualisation();		
			//semaphoreValues();
			
			//wake up producer thread, if producer was sleeping
			//if empty value is between 0 and (BOUNDED_BUFFER_SIZE-1), set full from 1 to 0 (enable producer)

			sem_getvalue(&empty, &emptyValue);
			sem_getvalue(&full, &fullValue);
			
			if(emptyValue >= 0 && emptyValue <= (BOUNDED_BUFFER_SIZE-1) && fullValue == 1)
				{
				sem_wait(&full);
				}

			if(emptyValue == BOUNDED_BUFFER_SIZE)
				{
				sem_wait(&full);
				}			
			}
		}
	}

void visualisation()
	{
	int i, j;
	int index = 0;

	for(i=0; i<BOUNDED_BUFFER_SIZE; i++)
		{
		if (boundedBuffer[i] == 0)
			{
			break;
			}
		else 
			{
			index++;
			}
		}
	
	printf("iTID = %u, iIndex = %d\t", pthread_self(), index);

	for(j=0; j<index; j++)
		{ 
		printf("*");
		}		

	printf("\n");
	}

void semaphoreValues()
	{
	sem_getvalue(&sync, &syncValue);
	sem_getvalue(&empty, &emptyValue);
	sem_getvalue(&full, &fullValue);

	printf("sync = %d, empty = %d, full = %d\n", syncValue, emptyValue, fullValue);
	}

int main()
	{
	int i;
	for(i=0; i<BOUNDED_BUFFER_SIZE; i++)
		{
		boundedBuffer[i] = 0;
		}

	sem_init(&sync, 0, 1);
	sem_init(&empty, 0, BOUNDED_BUFFER_SIZE); 	//counting semaphore
	sem_init(&full, 0, 1);

	pthread_t prodThread, con_threads[NUMBER_OF_CONSUMERS];
		
	int x;
	for(x=0; x<NUMBER_OF_CONSUMERS; x++)
		{
		pthread_create(&con_threads[x], NULL, consumer, NULL);
		}

	pthread_create(&prodThread, NULL, producer, NULL);

	pthread_join(prodThread, NULL);

	int y;
	for(y=0; y<NUMBER_OF_CONSUMERS; y++)
		{
		while(elementsConsumed != NUMBER_OF_JOBS);
		pthread_cancel(con_threads[y]);
		}

	semaphoreValues();
	 
	return 0;
	}