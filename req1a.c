#include <stdio.h> 
#include <sys/time.h>
 
#define NUMBER_OF_JOBS 1000
#define JOB_INDEX 0 
#define BURST_TIME 1 
#define REMAINING_TIME 2 
#define PRIORITY 3

int aiJobs[NUMBER_OF_JOBS][4]; 
int times[NUMBER_OF_JOBS][3];

void generateJobs(); 
void printJob(int iId, int iBurstTime, int iRemainingTime, int iPriority); 
void printJobs(); 
long int getDifferenceInMilliSeconds(struct timeval start, struct timeval end);
void simulateJob(int iTime);
void simulateJobFCFS();
void averageValuesFCFS();

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

void simulateJobFCFS() 
	{
	int i;
	int responseTime = 0;
	int turnaroundTime = 0;

	for (i=0; i<NUMBER_OF_JOBS; i++)
		{
		responseTime = turnaroundTime;

		times[i][0] = i;
		times[i][1] = responseTime;

		simulateJob(aiJobs[i][BURST_TIME]);

		turnaroundTime = responseTime + aiJobs[i][BURST_TIME];
		times[i][2] = turnaroundTime;

		printf("JOB ID = %d, Response Time = %d, Turnaround Time = %d\n", i, times[i][1], times[i][2]);
		}
	}

void averageValuesFCFS()
	{
	double avgResponse = 0;
	double avgTurnaround = 0;
	
	int j;
	for (j=0; j<NUMBER_OF_JOBS; j++)
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
	simulateJobFCFS();
	averageValuesFCFS();
	return 0;
	}
