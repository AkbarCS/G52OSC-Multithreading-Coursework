#include <stdio.h> 
#include <stdlib.h>
#include <semaphore.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h> 

#define NUMBER_OF_JOBS 1000
#define BOUNDED_BUFFER_SIZE 100
#define NUMBER_OF_CONSUMERS 4

#define JOB_INDEX 0 
#define BURST_TIME 1 
#define REMAINING_TIME 2 
#define ELEMENT_PRESENT 3

sem_t sync, empty, full; 

int syncValue, emptyValue, fullValue; 

int elementsProduced = 0;	//needed to assign job ids
int elementsConsumed = 0;	//number of jobs removed

int aiJobs[BOUNDED_BUFFER_SIZE][4]; 		//see defines lines 12 to 15
int threadIDArray[NUMBER_OF_CONSUMERS][3];	//0 is thread index number, 1 is startTime, 2 is time after job is finished
int startTime[NUMBER_OF_JOBS];

void *producer(void *p); 
void *consumer(void *p);
long int getDifferenceInMilliSeconds(struct timeval start, struct timeval end);
void simulateJob(int iTime);
void averageValuesFCFS();
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
				if(aiJobs[j][3] == 0)
					{
					aiJobs[j][0] = elementsProduced;
					aiJobs[j][1] = rand() % 99 + 1;;
					aiJobs[j][2] = aiJobs[j][1];
					aiJobs[j][3] = 1;
					elementsProduced++;
					sem_wait(&empty);
					break;
					}
				}	
			
			sem_post(&sync);
			
			sem_getvalue(&empty, &emptyValue);
			
			printf("Producing: JOB ID = %d, Burst Time = %d, Remaining Time = %d (jobs produced = %d, jobs in buffer = %d)\n", aiJobs[j][0], aiJobs[j][1], aiJobs[j][2], (elementsProduced), (BOUNDED_BUFFER_SIZE-emptyValue));
			
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
			
			if(aiJobs[0][3] == 1)
				{
				aiJobs[0][3] = 0;
				elementsConsumed++;
				sem_post(&empty);
				
				int currentThreadNumber = *(int*)p;
			
				threadIDArray[currentThreadNumber][1] = threadIDArray[currentThreadNumber][2];
				startTime[elementsConsumed-1] = threadIDArray[currentThreadNumber][2];
					
				sem_getvalue(&empty, &emptyValue);
				
				printf("Thread %d removes: JOB ID = %d, Burst Time = %d, Start Time = %d (jobs removed = %d, jobs in buffer = %d)\n", 
				currentThreadNumber+1, aiJobs[0][JOB_INDEX], aiJobs[0][BURST_TIME], threadIDArray[currentThreadNumber][1], elementsConsumed, (BOUNDED_BUFFER_SIZE-emptyValue));
				
				simulateJob(aiJobs[0][1]);
				threadIDArray[currentThreadNumber][2] += aiJobs[0][1];
				
				int j;
				for(j=0; j<BOUNDED_BUFFER_SIZE; j++)
					{
					aiJobs[j][0] = aiJobs[j+1][0];	
					aiJobs[j][1] = aiJobs[j+1][1];	
					aiJobs[j][2] = aiJobs[j+1][2];
					aiJobs[j][3] = aiJobs[j+1][3];	
					}
				}

			sem_post(&sync); 	
			
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
	
long int getDifferenceInMilliSeconds(struct timeval start, struct timeval end)
	{
	int iSeconds = end.tv_sec - start.tv_sec;
	int iUSeconds = end.tv_usec - start.tv_usec;
 	long int mtime = (iSeconds * 1000 + iUSeconds / 1000.0);
	return mtime;
	}

void simulateJob(int iTime)	//milliseconds
	{
	long int iDifference = 0;
	struct timeval startTime, currentTime;
	gettimeofday(&startTime, NULL);
	do
		{	
		gettimeofday(&currentTime, NULL);
		iDifference = getDifferenceInMilliSeconds(startTime, currentTime);
		} while(iDifference < iTime);
	}
	
void averageValuesFCFS()
	{
	float avgStartTime = 0;	
	int x;
	for(x=0; x<NUMBER_OF_JOBS; x++)
		{
		avgStartTime += startTime[x];
		}
		
	printf("Average Start Time = %.9f\n", avgStartTime/NUMBER_OF_JOBS);
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
	sem_init(&sync, 0, 1);
	sem_init(&empty, 0, BOUNDED_BUFFER_SIZE); 	//counting semaphore
	sem_init(&full, 0, 1);

	pthread_t prodThread, con_threads[NUMBER_OF_CONSUMERS];
		
	int x;
	for(x=0; x<NUMBER_OF_CONSUMERS; x++)
		{
		threadIDArray[x][0] = x;
		int *arg = malloc(sizeof(*arg));
		*arg = x;
		pthread_create(&con_threads[x], NULL, consumer, arg);
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
	averageValuesFCFS();
	 
	return 0;
	}