#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMCEP                  6
#define N_FRAME			1

static int min(int x, int y) 
{ 
	return x < y ? x:y;
}

static int max(int x, int y) 
{ 
	return x > y ? x:y;
}

int min_fun(int a, int b, int c)
{
	int min;
	return c<(min=a<b?a:b)?c:min;
}

#define INF 32767
#
int mConstraint = NUMCEP*N_FRAME/10;
int dtw[NUMCEP*N_FRAME][NUMCEP*N_FRAME];
int idtw_distance(short *mfcc_val, short *mfcc_train)
{
	int cost = 0;
	int icnt = 0,jcnt = 0;
	int ret = 0,Best = 0;

	dtw[0][0] = 0;
	for(icnt = 1; icnt < NUMCEP*N_FRAME; icnt++)
	{
		for(jcnt = max(0, icnt - mConstraint); jcnt < min(NUMCEP*N_FRAME, icnt + mConstraint + 1); ++jcnt)
		{
			if(icnt > 0)
			{
				Best = dtw[icnt - 1][jcnt];
			}

			if(jcnt > 0)
			{
				Best = min(Best,dtw[icnt][jcnt - 1]);
			}

			if((icnt > 0) && (jcnt > 0))
			{
				Best = min(Best, dtw[icnt - 1][jcnt - 1]);
			}

			if((icnt == 0)&& (jcnt == 0))
			{
				dtw[icnt][jcnt] = abs(mfcc_val - mfcc_train);
			}
			else
			{

				dtw[icnt][jcnt] = Best + abs(mfcc_val - mfcc_train);
				ret = dtw[icnt][jcnt];
			}
		}
	}

	//return dtw[NUMCEP*N_FRAME - 1][NUMCEP*N_FRAME - 1];
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
#if 0
	char *fileInA   = argv[1];
	char *fileInB   = argv[2];
	FILE *inFpA  = fopen(fileInA,"r");
	if(inFpA == NULL)
	{   
		fprintf(stderr, "failed to open pcm\n");
		return -1; 
	}
	FILE *inFpB  = fopen(fileInB,"r");
	if(inFpB == NULL)
	{
		fprintf(stderr, "failed to open pcm\n");
		return -1; 
	}

	int tempSize = NUMCEP*(N_FRAME + 1);
	int pcmLen = NUMCEP*N_FRAME;
	short *inA  = (short*)calloc(tempSize, sizeof(short));
	short *inB  = (short*)calloc(tempSize, sizeof(short));

	pcmLen = fread(inA, sizeof(short), tempSize, inFpA);
	pcmLen = fread(inB, sizeof(short), tempSize, inFpB);
#endif
	//int arraya[] = {3, 4, 5, 5, 5, 4}; 
	int arraya[] = {1, 2, 3, 4, 5, 5}; 
	int arrayb[] = {1, 2, 3, 4, 5, 5};
	int length = sizeof(arraya)/sizeof(arraya[0]);

	//short inTest[NUMCEP*N_FRAME],inNum[NUMCEP*N_FRAME];
	//memset(inTest,0x0,NUMCEP*N_FRAME*sizeof(short));
	//memset(inNum,0x1,NUMCEP*N_FRAME*sizeof(short));
	//int distances = idtw_distance(inA,inB);
	int distances = idtw_distance(arraya,arrayb);
	//int distances = dtw_distance(inTest,inNum);
	printf("the distances between two mfcc val is:%d \n",distances);
#if 0
	free(inA);
	free(inB);
	fclose(inFpA);
	fclose(inFpB);
#endif
	return 0;
}
