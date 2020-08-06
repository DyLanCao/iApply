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
//#include "../user/user.h"

#define MAXSTRLEN 1024
#define mu 0.0002f             //convergence rate

#define M 64

//double H[M] = { 1, 0.5, 0.25, 0.125, 0.0625 };    //the main system
double H[M] = { 0.0625, 0.125, 0.25, 0.5, 1 };      //we need inverse of main system to convolution

int tempSize_16k = 160;

int X[M] = { 0.0 };
double W[M] = { 0.0 };
double D[160],Y[160],E[160];
double stepsize = 0.0;

double Desired_value[160];

void init_desired_value(void)
{
    for(int icnt = 0; icnt < tempSize_16k; icnt++)
        Desired_value[icnt] = rand()%100 - 50;
    
    for(int cnt =0 ; cnt < M; cnt++)
    {
        W[cnt] = 0.0;
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

    init_desired_value();

    FILE *inFp  = fopen(fileIn,"r");
    FILE *outFp = fopen(fileOut,"w");
    if(inFp == NULL || outFp == NULL)
    {
        fprintf(stderr, "failed to open pcm\n");
        return -1;
    }

    short *in_16k  = (short*)calloc(tempSize_16k, sizeof(short));
    short *out_16k = (short*)calloc(tempSize_16k, sizeof(short));
    int pcmLen = tempSize_16k;
    
    start = clock();

    while(pcmLen > 0)
    {

        pcmLen = fread(in_16k, sizeof(short), tempSize_16k, inFp);

	    //memcpy(out_16k,in_16k,pcmLen*sizeof(short));
        for(int frame_cnt = M; frame_cnt <= tempSize_16k; frame_cnt++)
        {

            for(int tmp_cnt = frame_cnt - 1; tmp_cnt >= frame_cnt - M;tmp_cnt--)
            {
                X[frame_cnt - tmp_cnt - 1] = in_16k[tmp_cnt];
            }


            for(int icnt = 0; icnt < M; icnt++)
            {
                out_16k[frame_cnt -1] += W[icnt]*X[icnt];
            }

            D[frame_cnt - 1] = rand()%100 - 50;

            E[frame_cnt - 1] = (D[frame_cnt  - 1] - (double)out_16k[frame_cnt - 1]);

            //printf(" aE:%lf D:%lf O:%d\n\t ",E[frame_cnt - 1],D[frame_cnt - 1],out_16k[frame_cnt - 1]);
            // printf(" E:%d O:%d \n",out_16k[frame_cnt - 1],out_16k[frame_cnt - 1]);
            // printf(" E:%f O:%f \n",out_16k[frame_cnt - 1],out_16k[frame_cnt - 1]);

            // short x = 3;
            // printf("%d ### %lf\n",x, (double)x);
            
            for(int icnt =0 ; icnt < M; icnt++)
            {

                stepsize += (X[icnt]*X[icnt]);
            }

            stepsize = 1.0/stepsize;


            for(int icnt =0; icnt < M; icnt++)
            {
                W[icnt] = W[icnt] + stepsize*E[frame_cnt -1]*X[icnt];
                //printf("W:%lf \n\t ",W[icnt]);
            }

        }
        
        count++;


        pcmLen = fwrite(out_16k, sizeof(short), pcmLen, outFp);
    }


    finish = clock();
    printf("count:%d speed time:%f \n\t",count,(double)(finish - start) / CLOCKS_PER_SEC);

    fclose(inFp);
    fclose(outFp);
    free(in_16k);
    free(out_16k);

    return 0;
}
