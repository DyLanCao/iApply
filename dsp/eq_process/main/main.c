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
#include "../app/speech_eq.h"
#include <math.h>


#include "../app/speech_eq.h"

IIR_RUN_CFG_T speech_rx_eq_state;

extern const EqConfig speech_rx_eq_cfg;

#define MAXSTRLEN 1024

void speech_rx_eq_init(enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T sample_bits, enum AUD_CHANNEL_NUM_T chan_num)
{

   speech_iir_open(&speech_rx_eq_state, sample_rate, sample_bits, chan_num);

   speech_iir_set_cfg(&speech_rx_eq_state, &speech_rx_eq_cfg);
}

const EqConfig speech_rx_eq_cfg = {
    .gain0 = 0,
    .gain1   = 0.f,
    .num    = 1,
    .param = {
        // high pass rang form 0.01 to 0.9 amplitude convert to value
        //{SPEECH_PEAK, -15.0, 6000.0, 0.1},
        //{SPEECH_HIGH_PASS, 0, 2000.0, 0.1},
         {SPEECH_LOW_PASS, 0, 6000, 0.2},
        // peak range from 0
       // {SPEECH_PEAK, 6.0, 300, 1},

    },
};

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

    speech_rx_eq_init((enum AUD_SAMPRATE_T)sampleRate, AUD_BITS_16, AUD_CHANNEL_NUM_1);

    start = clock();

    while(pcmLen > 0)
    {

        pcmLen = fread(in_16k, sizeof(short), tempSize_16k, inFp);
	
	memset((unsigned char*)in_16k,0x01,2*pcmLen);
    	speech_iir_run(&speech_rx_eq_state, (unsigned char*)in_16k, pcmLen);
	buf_txt(in_16k,pcmLen);
	memcpy(out_16k,in_16k,pcmLen*sizeof(short));
	count++;
        pcmLen = fwrite(out_16k, sizeof(short), pcmLen, outFp);
    }


    finish = clock();
    printf("count:%d speed time:%f \n",count,(double)(finish - start) / CLOCKS_PER_SEC);

    speech_iir_close(&speech_rx_eq_state);

    fclose(inFp);
    fclose(outFp);
    free(in_16k);
    free(out_16k);

    return 0;
}
