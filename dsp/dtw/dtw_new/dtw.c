#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMCEP                  40
#define N_FRAME			98

int min( int x, int y, int z ) {
	if( ( x <= y ) && ( x <= z ) ) return x;
	if( ( y <= x ) && ( y <= z ) ) return y;
	if( ( z <= x ) && ( z <= y ) ) return z;
}

#define SIZE 10
#define INF 32767


int mGamma[SIZE][SIZE];
int temp[SIZE + 1][SIZE + 1];

int max_fun(int a, int b)
{
	int max;
	return max=a>b?a:b;
}

int dtw_run(int *v, int *w,int size) 
{
	int cost;
	int ret;

	for( int i = 0; i < size + 1; i++ ) 
	  for(int j = 0; j < size + 1; j++)
	  {
		  temp[i][j] = 0;
	  }

	for( int i = 1; i < size; i++ ) {
		temp[0][i] = INF;
	}

	for( int i = 1; i < size; i++ ) {
		temp[i][0] = INF;
	}

	mGamma[0][0] = 0;

	for( int i = 0; i < size; i++ ) 
	  for(int j = 0; j < size; j++)
	  {
		  temp[i + 1][j + 1] = mGamma[i][j] = abs(v[i] - w[j]);
		  //temp[i + 1][j + 1] = mGamma[i][j] = v[j];
		  printf("ret...:%d \n",mGamma[i][j]);
	  }


	for( int i = 0; i < size; i++ ) {
		for( int j = 0; j < size; j++ ) 
		{
			mGamma[i][j] += min( temp[i][j], temp[i][j+1], temp[i+1][j] );
			ret = mGamma[i][j];
			temp[i + 1][j + 1] = mGamma[i][j];
			printf("ret:%d ",ret);
		}
	}

	ret = max_fun(mGamma[size - 1][size - 2],mGamma[size - 2][size - 1]);
	printf("mGamma:%d ",mGamma[size - 1][size - 2]);
	printf("mGammb:%d ",mGamma[size - 2][size - 1]);
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
	//int arraya[] = {3, 4, 5, 5, 5, 4};
	//int arrayb[] = {1, 2, 3, 4, 5, 5};
	int arraya[] = {-100, 0, 1, 1, 2, 4, 200, 1, -2, 0};
	int arrayb[] = {1000, 10, 10, 20, 2, 2, 2, 3, 2, 100};

	int length = sizeof(arraya)/sizeof(arraya[0]);
	//int distances = dtw_distance(arraya,arrayb,length);
	int distances = dtw_run(arraya,arrayb,length);
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
