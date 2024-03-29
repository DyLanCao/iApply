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

#define M 5

//double H[M] = { 1, 0.5, 0.25, 0.125, 0.0625 };    //the main system
double H[M] = { 0.0625, 0.125, 0.25, 0.5, 1 };      //we need inverse of main system to convolution

int tempSize_16k = 160;

int X[M] = { 0.0 };
double W[M] = { 0.0 };
double D,Y,E;

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
        for(int frame_cnt = 0; frame_cnt < tempSize_16k; frame_cnt++)
        {

            for(int tmp_cnt = frame_cnt; tmp_cnt > frame_cnt - M;tmp_cnt--)
            {
                if(tmp_cnt >= 0)
                {
                    X[M + (tmp_cnt - frame_cnt) - 1] = in_16k[tmp_cnt];
                }
                else
                    break;
            }

            D = Desired_value[frame_cnt];
            //D = 32.5;
            Y = 0;

            for(int icnt = 0; icnt < M; icnt++)
            {
                Y +=(W[icnt]*X[icnt]);
                //printf("W:i:%d W:%f \n\t",icnt,W[icnt]);
                //Y +=X[icnt];
            }

            E = D - Y;
            //E = 1;
            
            printf("Ee:Y:%f E:%f \n\t",Y,E);
            for(int icnt =0 ; icnt < M; icnt++)
                W[icnt] = W[icnt] + (mu * E * X[icnt]);

        }

    /*
        for(int icnt = 0; icnt < tempSize_16k; icnt++)
        {
            for(int tmp_sample = 0;tmp_sample < M; tmp_sample++ )
            {
                out_16k[icnt] += in_16k[icnt]*W[tmp_sample];

            }
        }
        
        for(int icnt = 0; icnt < M; icnt++)
            printf(" i:%d w:%f \n\t",icnt,W[icnt]);

      */

	    if(count++>1)
        {
            break;
        }

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
