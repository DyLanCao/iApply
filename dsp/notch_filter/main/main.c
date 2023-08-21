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
#include "../user/user.h"
#include "../app/notch_filter.h"
#include <math.h>




void buf_txt(short *buf,int len)
{
        for(int i=0; i < len; i++)
        {
                printf("%d,",buf[i]);
        }
}

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
    int status;

   clock_t start, finish;
   double  duration;
   int count;


    FILE *inFp  = fopen(fileIn,"r");
    FILE *outFp = fopen(fileOut,"w");
    if(inFp == NULL || outFp == NULL)
    {
        fprintf(stderr, "failed to open pcm\n");
        return -1;
    }

    int tempSize_16k = 160;
    short *in_16k  = (short*)calloc(tempSize_16k, sizeof(short));
    short *out_16k = (short*)calloc(tempSize_16k, sizeof(short));
    int pcmLen = tempSize_16k;
    
    int sampleRate = 16000;

    br_iir_init(sampleRate,1,0.1,2,1000);

    start = clock();

    while(pcmLen > 0)
    {

        pcmLen = fread(in_16k, sizeof(short), tempSize_16k, inFp);
	
	memset((unsigned char*)in_16k,0x01,2*pcmLen);

	memcpy(out_16k,in_16k,pcmLen*sizeof(short));
	count++;
        pcmLen = fwrite(out_16k, sizeof(short), pcmLen, outFp);
    }


    finish = clock();
    printf("count:%d speed time:%f \n",count,(double)(finish - start) / CLOCKS_PER_SEC);


    fclose(inFp);
    fclose(outFp);
    free(in_16k);
    free(out_16k);

    return 0;
}
