#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMCEP                  40
#define N_FRAME			98

int min_fun(int a, int b, int c)
{
	int min;
	return c<(min=a<b?a:b)?c:min;
}

//int dtw[NUMCEP*N_FRAME][NUMCEP*N_FRAME];
int dtw[NUMCEP*N_FRAME][NUMCEP*N_FRAME];
int dtw_distance(short *mfcc_val, short *mfcc_train, int length)
{
	int cost = 0;
	int icnt = 0,jcnt = 0;
	int ret = 0;
	for(int i=0; i < length; i++)
	{
		dtw[i][0] = 0;
	}

	for(int j=0; j < length; j++)
	{
		dtw[0][j] = 0;
	}

	dtw[0][0] = 0;
	for(icnt = 1; icnt < length; icnt++)
	{
	  for(jcnt = 1; jcnt < length; jcnt++)
	  {
		  cost = abs(mfcc_val[icnt] - mfcc_train[jcnt]);
		  dtw[icnt][jcnt] = cost + min_fun(dtw[icnt - 1][jcnt],dtw[icnt][jcnt - 1],dtw[icnt - 1][jcnt - 1]);
		  ret = dtw[icnt][jcnt];
		  printf("ret:%d cost:%d \n",ret,cost);
	  }
	}

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
	/*
	short inTest[NUMCEP*N_FRAME],inNum[NUMCEP*N_FRAME];

	for(int icnt = 0; icnt < NUMCEP*N_FRAME; icnt++)
	{
		inTest[icnt] = 12;
		inNum[icnt] = -1;
	}
	*/
	//memset(inTest,0x0,NUMCEP*N_FRAME*sizeof(short));
	//memset(inNum,0x1,NUMCEP*N_FRAME*sizeof(short));
	//int distances = dtw_distance(inA,inB);
	int arraya[] = {3, 4, 5, 5, 5, 4};
	int arrayb[] = {1, 2, 3, 4, 5, 5};
	int length = sizeof(arraya)/sizeof(arraya[0]);
	int distances = dtw_distance(arraya,arrayb,length);
	//int distances = dtw_distance(inTest,inNum);
	printf("the distances between two mfcc val is:%d length:%d \n",distances,length);
	printf("the avg distances between two mfcc val is:%d \n",distances/(NUMCEP*N_FRAME));

#if 0
	free(inA);
	free(inB);
	fclose(inFpA);
	fclose(inFpB);
#endif
	return 0;
}
