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

#include "PDM_SoftCICfilter.h"

#define MAXSTRLEN 1024

unsigned char sink_buff[] = {0,1,0,1,0,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,1,0,1};

int main(int argc, char *argv[])
{

   clock_t start, finish;
   double  duration;
   int count = 0;

   FILE *fd_in, *fd_out;

   fd_out = fopen("filtered.wav", "w");

    test_func();

    initializeCicFilterStruct(3, DECIMATION_M, &PDM_st);
  
    start = clock();
    
    printf("sink_buff:%d \n",sizeof(sink_buff)/sizeof(sink_buff[0]));
   
    for(int cnt = 0; cnt < PDM_SAMPLE_SIZE * DECIMATION_M / 8; cnt +=100)
    {
	memcpy(PDM_RawBits + cnt,sink_buff,100);
    }

    executeCicFilter((uint8_t*) PDM_RawBits, PDM_SAMPLE_SIZE * DECIMATION_M,(int32_t*) PDM_Filtered_int32, &PDM_st);

    for (uint32_t i = 0; i < PDM_SAMPLE_SIZE; i++)
    {

    	printf(" cnt:%d  val: %d \n",i,PDM_Filtered_int32[i]);
    }

   fwrite(PDM_Filtered_int32, sizeof(int), PDM_SAMPLE_SIZE, fd_out);

    finalizeCicFilterStruct(&PDM_st);

    finish = clock();
    printf("count:%d speed time:%f \n",count,(double)(finish - start) / CLOCKS_PER_SEC);

    fclose(fd_out);

    return 0;
}
