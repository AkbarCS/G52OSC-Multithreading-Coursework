#include <stdio.h> 
#include <stdlib.h>
#include <semaphore.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h> 

#define NUMBER_OF_JOBS 1000
#define BOUNDED_BUFFER_SIZE 100
#define NUMBER_OF_CONSUMERS 4
#define TIME_SLICE 25

#define JOB_INDEX 0 
#define BURST_TIME 1 
#define REMAINING_TIME 2
#define PRIORTY 3
#define RESPONDED 4
#define ELEMENT_PRESENT 5

sem_t sync, empty, full; 

int syncValue, emptyValue, fullValue; 

int elementsProduced = 0;	//needed to assign job ids
int elementsConsumed = 0;	//number of jobs removed

int aiJobs[BOUNDED_BUFFER_SIZE][6]; 		//see defines lines 12 to 17
int threadIDArray[NUMBER_OF_CONSUMERS][3];	//0 is thread index number, 1 is startTime, 2 is time after job is finished
int startTime[NUMBER_OF_JOBS];

void *producer(void *p); 
void *consumer(void *p);
long int getDifferenceInMilliSeconds(struct timeval start, struct timeval end);
void simulateJob(int iTime);
void insertionSort();
void averageValuesPriortyQueue();
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
				if(aiJobs[j][ELEMENT_PRESENT] == 0)
					{
					aiJobs[j][JOB_INDEX] = elementsProduced;
					aiJobs[j][BURST_TIME] = rand() % 99 + 1;
					aiJobs[j][REMAINING_TIME] = aiJobs[j][1];
					aiJobs[j][PRIORTY] = (rand() % 9) + 1;
					aiJobs[j][RESPONDED] = 0;
					aiJobs[j][ELEMENT_PRESENT] = 1;
					elementsProduced++;
					sem_wait(&empty);
					break;
					}
				}	
			
			sem_post(&sync);
			
			sem_getvalue(&empty, &emptyValue);
												
			printf("Producing: JOB ID = %d, Burst Time = %d, Remaining Time = %d, Priority = %d (jobs produced = %d, jobs in buffer = %d)\n", 
			aiJobs[j][JOB_INDEX], aiJobs[j][BURST_TIME], aiJobs[j][REMAINING_TIME], aiJobs[j][PRIORTY], (elementsProduced), (BOUNDED_BUFFER_SIZE-emptyValue));
						
			sem_getvalue(&empty, &emptyValue);
			sem_getvalue(&full, &fullValue);
				
			//if the number of empty slots is between 0 and BOUNDED_BUFFER_SIZE-1 (i.e. there is at least one element),
			//set full from 0 to 1 (enable consumer) 
			if((emptyValue >= 0 && emptyValue <= BOUNDED_BUFFER_SIZE-1) && fullValue == 0)
				{
				sem_post(&full);
				} 			
				
			if(emptyValue == 0 || emptyValue == 1)
				{
				while(emptyValue != BOUNDED_BUFFER_SIZE-2)	
					{
					sem_getvalue(&empty, &emptyValue);
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
			
			//sort buffer/element with highest priority (priority 1)
			insertionSort();
			
			if(aiJobs[0][ELEMENT_PRESENT] == 1)
				{				
				int currentThreadNumber = *(int*)p;

				if(aiJobs[0][RESPONDED] == 0)
					{
					aiJobs[0][RESPONDED] == 1;
					startTime[elementsConsumed] = threadIDArray[currentThreadNumber][2];	
					}
				
				if(aiJobs[0][REMAINING_TIME] <= TIME_SLICE && aiJobs[0][REMAINING_TIME] != 0)
					{
					aiJobs[0][ELEMENT_PRESENT] = 0;
					
					threadIDArray[currentThreadNumber][1] = threadIDArray[currentThreadNumber][2];
					
					elementsConsumed++;
					sem_post(&empty);
					
					sem_getvalue(&empty, &emptyValue);
					
					printf("Thread %d removes: JOB ID = %d, Re-Start Time = %d, End Time = %d, Priority = %d (jobs removed = %d, jobs in buffer = %d)\n", 
					currentThreadNumber+1, aiJobs[0][JOB_INDEX], threadIDArray[currentThreadNumber][1], (threadIDArray[currentThreadNumber][1] + aiJobs[0][REMAINING_TIME]), aiJobs[0][PRIORTY], elementsConsumed, (BOUNDED_BUFFER_SIZE-emptyValue));
				
					simulateJob(aiJobs[0][REMAINING_TIME]);
					threadIDArray[currentThreadNumber][2] += aiJobs[0][REMAINING_TIME];
						
					int i;
					for(i=0; i<BOUNDED_BUFFER_SIZE; i++)
						{
						aiJobs[i][0] = aiJobs[i+1][0];	
						aiJobs[i][1] = aiJobs[i+1][1];	
						aiJobs[i][2] = aiJobs[i+1][2];
						aiJobs[i][3] = aiJobs[i+1][3];	
						aiJobs[i][4] = aiJobs[i+1][4];
						aiJobs[i][5] = aiJobs[i+1][5];
						}
					}
				else if(aiJobs[0][REMAINING_TIME] != 0)
					{
					threadIDArray[currentThreadNumber][1] = threadIDArray[currentThreadNumber][2];
	
					sem_getvalue(&empty, &emptyValue);
	
					simulateJob(TIME_SLICE);
					threadIDArray[currentThreadNumber][2] += TIME_SLICE;
					
					if(aiJobs[0][RESPONDED] == 2)
						{
						printf("Thread %d restarts and processes: JOB ID = %d, Re-Start Time = %d, Remaining Time = %d, Priority = %d (jobs removed = %d, jobs in buffer = %d)\n", 
						currentThreadNumber+1, aiJobs[0][JOB_INDEX], threadIDArray[currentThreadNumber][1], aiJobs[0][REMAINING_TIME], aiJobs[0][PRIORTY], elementsConsumed, (BOUNDED_BUFFER_SIZE-emptyValue));	
						}
					else
						{
						printf("Thread %d processes: JOB ID = %d, Start Time = %d, Remaining Time = %d, Priority = %d (jobs removed = %d, jobs in buffer = %d)\n", 
						currentThreadNumber+1, aiJobs[0][JOB_INDEX], threadIDArray[currentThreadNumber][1], aiJobs[0][REMAINING_TIME], aiJobs[0][PRIORTY], elementsConsumed, (BOUNDED_BUFFER_SIZE-emptyValue));
						}
					
					aiJobs[0][REMAINING_TIME] -= 25;
					aiJobs[0][RESPONDED] = 2;
					
					//add job to the end of the buffer					
					int j, j2;
					for(j=0; j<BOUNDED_BUFFER_SIZE; j++)
						{
						if(aiJobs[j][ELEMENT_PRESENT] == 0)	
							{
							aiJobs[j][0] = aiJobs[0][0];	
							aiJobs[j][1] = aiJobs[0][1];	
							aiJobs[j][2] = aiJobs[0][2];
							aiJobs[j][3] = aiJobs[0][3];	
							aiJobs[j][4] = aiJobs[0][4];
							aiJobs[j][5] = aiJobs[0][5];
							break;
							}
						}
					
					//shift elements to the left
					for(j2=0; j2<BOUNDED_BUFFER_SIZE; j2++)
						{
						aiJobs[j2][0] = aiJobs[j2+1][0];	
						aiJobs[j2][1] = aiJobs[j2+1][1];	
						aiJobs[j2][2] = aiJobs[j2+1][2];
						aiJobs[j2][3] = aiJobs[j2+1][3];	
						aiJobs[j2][4] = aiJobs[j2+1][4];
						aiJobs[j2][5] = aiJobs[j2+1][5];	
						}
					}
				}

			sem_post(&sync); 	
			
			sem_getvalue(&empty, &emptyValue);
			sem_getvalue(&full, &fullValue);
					
			//if the number of empty slots is between 1 and BOUNDED_BUFFER_SIZE, set full from 1 to 0 (enable producer)
			if(emptyValue >= 1 && emptyValue <= (BOUNDED_BUFFER_SIZE) && fullValue == 1)
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
	
void insertionSort()
	{
	sem_getvalue(&empty, &emptyValue);
	int c, d;
	int temp[2][6];
	
	for (c = 1; c <= (BOUNDED_BUFFER_SIZE-emptyValue) - 1; c++) 
		{
		d = c; 
		while ( d > 0 && aiJobs[d][PRIORTY] < aiJobs[d-1][PRIORTY]) 
			{
			temp[0][0] = aiJobs[d][0];
			temp[0][1] = aiJobs[d][1];
			temp[0][2] = aiJobs[d][2];
			temp[0][3] = aiJobs[d][3];
			temp[0][4] = aiJobs[d][4];
			temp[0][5] = aiJobs[d][5];

			temp[1][0] = aiJobs[d-1][0];
			temp[1][1] = aiJobs[d-1][1];
			temp[1][2] = aiJobs[d-1][2];
			temp[1][3] = aiJobs[d-1][3];
			temp[1][4] = aiJobs[d-1][4];
			temp[1][5] = aiJobs[d-1][5];
			
			aiJobs[d-1][0] = temp[0][0];
			aiJobs[d-1][1] = temp[0][1];
			aiJobs[d-1][2] = temp[0][2];
			aiJobs[d-1][3] = temp[0][3];
			aiJobs[d-1][4] = temp[0][4];
			aiJobs[d-1][5] = temp[0][5];
					
			aiJobs[d][0] = temp[1][0];
			aiJobs[d][1] = temp[1][1];
			aiJobs[d][2] = temp[1][2];
			aiJobs[d][3] = temp[1][3];
			aiJobs[d][4] = temp[1][4];
			aiJobs[d][5] = temp[1][5];
			
			d--;
			}
		}	
	} 
	
void averageValuesPriortyQueue()
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
		pthread_join(con_threads[y], NULL);
		}
		
	semaphoreValues();
	averageValuesPriortyQueue();
	 
	return 0;
	}
