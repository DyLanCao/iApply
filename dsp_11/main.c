/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015年12月03日 16时29分36秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "echo.h"

#define MAXSTRLEN 1024

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "Usage: <in> <out>\n");
        return -1;
    }
	char *fileIn  = argv[1];
	char *fileOut = argv[2];
    //ZeusFront front;
    int status = 0;

   clock_t start, finish;
   double  duration;
   int count = 0;
   // status = front.FrontInit(16000, 16, 6, 3, 3);
     status = echo_init();
    if(status != 0)
    {
        fprintf(stderr, "failed to init\n");
        return -1;
    }
	FILE *inFp  = fopen(fileIn,"r");
    FILE *outFp = fopen(fileOut,"w");
    if(inFp == NULL || outFp == NULL)
    {
        fprintf(stderr, "failed to open pcm\n");
        return -1;
    }
    int tempSize = 160;
    short *in  = (short*)calloc(tempSize, sizeof(short));
    short *out = (short*)calloc(tempSize, sizeof(short));
    int pcmLen = tempSize;
    
    start = clock();

    while(pcmLen > 0)
    {
        pcmLen = fread(in, sizeof(short), tempSize, inFp);
       // front.FrontProc(in, out, pcmLen);
		//printf("count:%d speed time:%f",count,(double)(finish - start) / CLOCKS_PER_SEC);
	    echo_process(in, out, pcmLen);
        pcmLen = fwrite(out, sizeof(short), pcmLen, outFp);
		count++;
    }
    finish = clock();
    printf("count:%d speed time:%f",count,(double)(finish - start) / CLOCKS_PER_SEC);

    fclose(inFp);
    fclose(outFp);
    free(in);
    free(out);
    return 0;
}
