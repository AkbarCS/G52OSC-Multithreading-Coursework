#include <stdio.h> 
#include <semaphore.h> 
#include <pthread.h> 

#define NUMBER_OF_JOBS 1000
#define BOUNDED_BUFFER_SIZE 100

sem_t sync, empty, full; 

int emptyTest, fullTest, syncValue, emptyValue, fullValue; 

int elementsProduced = 0;
int elementsConsumed = 0;

char boundedBuffer[BOUNDED_BUFFER_SIZE];

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

			int j;
			for(j=0; j<BOUNDED_BUFFER_SIZE; j++)
				{
				if(boundedBuffer[j] == 0)
					{
					boundedBuffer[j] = 42;
					elementsProduced++;
					break;
					}
				}
			
			sem_post(&sync);
			visualisation();
						
			//check if element exists in buffer
			//fullTest == 1 means an element exists in the buffer and fullTest == 0 means no element exists in buffer 
			fullTest = 0;
			int bufferFull = 1;		
	
			for(j=0; j<BOUNDED_BUFFER_SIZE; j++)
				{
				if(boundedBuffer[j] == 42)
					{
					fullTest = 1;
					}

				if(boundedBuffer[j] == 0)
					{
					bufferFull = 0;
					}
				} 

			//if there is at least one element in the buffer, wake up consumer			
			sem_getvalue(&full, &fullValue);
			sem_getvalue(&empty, &emptyValue); 

			if(fullTest == 1 && fullValue == 0 && emptyValue == 1)
				{
				sem_post(&full);
				sem_wait(&empty);
			
				//if buffer is full, put this thread to sleep
				if(bufferFull == 1)
					{
					sem_wait(&empty);
					}
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
				int j;
				for(j=0; j<BOUNDED_BUFFER_SIZE; j++)
					{
					int tmp = boundedBuffer[j+1];
					boundedBuffer[j] = tmp;			
					}
				}
		
			sem_post(&sync); 		
			visualisation();
			
			//check if empty space exists in the buffer
			//emptyTest == 1 means empty space exists in buffer and emptyTest == 0 means empty space does not exist in buffer 
			emptyTest = 0;
			int bufferEmpty = 1;			

			int j2;
			for(j2=0; j2<BOUNDED_BUFFER_SIZE; j2++)
				{
				if(boundedBuffer[j2] == 0)
					{
					emptyTest = 1;
					}

				if(boundedBuffer[j2] == 42)
					{
					bufferEmpty = 0;
					}
				} 
			
			//if there is at least one space in the buffer, wake up producer
			sem_getvalue(&full, &fullValue);
			sem_getvalue(&empty, &emptyValue); 

			if(emptyTest == 1 && fullValue == 1 && emptyValue == 0)
				{
				sem_post(&empty);
				sem_wait(&full);
				
				if(bufferEmpty == 1)
					{
					sem_wait(&full);
					}
				}
			}			
		}
	}

void visualisation()
	{
	int y = 0;
	int z = 0;
	int index = 0;

	for(y=0; y<BOUNDED_BUFFER_SIZE; y++)
		{
		if(boundedBuffer[y] == 0)
			{
			break;
			}
		else 
			{
			index++;
			}
		}
	
	printf("iIndex = %d\t", index);

	for(z=0; z<index; z++)
		{ 
		printf("*");
		}		
		
	printf("\n");
	}

int main()
	{
	int i;
	for(i=0; i<BOUNDED_BUFFER_SIZE; i++)
		{
		boundedBuffer[i] = 0;
		}

	sem_init(&sync, 0, 1);
	sem_init(&empty, 0, 1);
	sem_init(&full, 0, 0);

	pthread_t prodThread, conThread;

	pthread_create(&conThread, NULL, consumer, NULL);
	pthread_create(&prodThread, NULL, producer, NULL);

	pthread_join(conThread, NULL);
	pthread_join(prodThread, NULL);

	sem_getvalue(&sync, &syncValue);
	sem_getvalue(&empty, &emptyValue);
	sem_getvalue(&full, &fullValue);

	printf("sync = %d, empty = %d, full = %d\n", syncValue, emptyValue, fullValue);
 
	return 0;
	}
