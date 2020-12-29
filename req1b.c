#include <stdio.h> 
#include <sys/time.h>
 
#define NUMBER_OF_JOBS 1000
#define TIME_SLICE 25

#define JOB_INDEX 0 
#define BURST_TIME 1 
#define REMAINING_TIME 2 
#define PRIORITY 3
#define RESPONDED 4

int aiJobs[NUMBER_OF_JOBS][5]; 
int times[NUMBER_OF_JOBS][3];

void generateJobs(); 
void printJob(int iId, int iBurstTime, int iRemainingTime, int iPriority); 
void printJobs();
long int getDifferenceInMilliSeconds(struct timeval start, struct timeval end);
void simulateJob(int iTime);
void simulateJobPriorityQueue(); 
void averageValuesPriorityQueue();

void generateJobs() 
	{
	int i;
	srand(10);
	for(i = 0; i < NUMBER_OF_JOBS;i++)
		{
		aiJobs[i][JOB_INDEX] = i;
		aiJobs[i][BURST_TIME] = rand() % 99 + 1;
		aiJobs[i][REMAINING_TIME] = aiJobs[i][BURST_TIME];
		aiJobs[i][PRIORITY] = (rand() % 9) + 1;
		aiJobs[i][RESPONDED] = 0;
		}
	}

void printJob(int iId, int iBurstTime, int iRemainingTime, int iPriority) 
	{
	printf("Id = %d, Burst Time = %d, Remaining Time = %d, Priority = %d\n", iId, iBurstTime, iRemainingTime, iPriority);
	}

void printJobs() 
	{
	int i;
	printf("JOBS: \n");
	for(i = 0; i < NUMBER_OF_JOBS; i++)
		{
		printJob(aiJobs[i][JOB_INDEX], aiJobs[i][BURST_TIME], aiJobs[i][REMAINING_TIME], aiJobs[i][PRIORITY]);
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

void simulateJobPriorityQueue() 
	{
	int responseTime = 0;
	int turnaroundTime = 0;

	int i, j;

	for (i=0; i<10; i++)
		{
		int i2;
		int jobPrioritySame = 0;
		
		for (i2=0; i2<NUMBER_OF_JOBS; i2++)
			{
			if(aiJobs[i2][PRIORITY] == i)
				{
				jobPrioritySame++;
				}
			}

		int completed = 0;
		while(completed != jobPrioritySame)
			{
			for (j=0; j<NUMBER_OF_JOBS; j++)
				{
				if(aiJobs[j][PRIORITY] == i)
					{
					if (aiJobs[j][RESPONDED] == 0)
						{
						aiJobs[j][RESPONDED] = 1;
						times[j][1] = responseTime;
						}
					
					if(aiJobs[j][REMAINING_TIME] <= TIME_SLICE && aiJobs[j][REMAINING_TIME] != 0)
						{
						printf("Job ID: %d, Start Time: %d, End Time: %d, Priority: %d\n", aiJobs[j][JOB_INDEX], responseTime, 
						(responseTime + aiJobs[j][REMAINING_TIME]), aiJobs[j][PRIORITY]);

						simulateJob(aiJobs[j][REMAINING_TIME]);
						
						turnaroundTime += aiJobs[j][REMAINING_TIME];
						responseTime += aiJobs[j][REMAINING_TIME];

						aiJobs[j][REMAINING_TIME] -= aiJobs[j][REMAINING_TIME];

						times[j][2] = turnaroundTime;
						
						completed++;
						}
					else if(aiJobs[j][REMAINING_TIME] != 0)
						{
						if(aiJobs[j][RESPONDED] == 2)
							{
							printf("Job ID: %d, Re-Start Time: %d, Remaining Time: %d, Priority: %d\n", aiJobs[j][JOB_INDEX], responseTime, aiJobs[j][REMAINING_TIME], aiJobs[j][PRIORITY]);
							}
						else 
							{
							printf("Job ID: %d, Start Time: %d, Remaining Time: %d, Priority: %d\n", aiJobs[j][JOB_INDEX], responseTime, aiJobs[j][REMAINING_TIME], aiJobs[j][PRIORITY]);
							}	
						
						simulateJob(TIME_SLICE);

						aiJobs[j][REMAINING_TIME] -= TIME_SLICE;
						aiJobs[j][RESPONDED] = 2;
						
						responseTime += TIME_SLICE;
						turnaroundTime += TIME_SLICE;
						}
					} 
				}
			}
		}	
	}

void averageValuesPriorityQueue()
	{
	double avgResponse = 0;
	double avgTurnaround = 0;
	
	int j;
	for(j=0; j<NUMBER_OF_JOBS; j++)
		{
		avgResponse += times[j][1];
		avgTurnaround += times[j][2];
		}

	avgResponse = avgResponse/NUMBER_OF_JOBS;
	avgTurnaround = avgTurnaround/NUMBER_OF_JOBS;

	printf("Average Response Time = %.9f, Average Turnaround Time = %.9f\n", avgResponse, avgTurnaround);
	}

int main() 
	{
	generateJobs();
	printJobs();
	printf("Time Slice is: %d\n", TIME_SLICE);
	simulateJobPriorityQueue();
	averageValuesPriorityQueue();
	return 0;
	}
