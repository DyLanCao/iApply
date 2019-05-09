#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define DELAY_BUFSIZ ( 50 * 50U * 1024 )
#define MAX_ECHOS 7     /* 24 bit x ( 1 + MAX_ECHOS ) = */
/* 24 bit x 8 = 32 bit !!!      */

/* Private data for SKEL file */
struct priv_t{
	int     counter;
	int     num_delays;
	double  *delay_buf;
	float   in_gain, out_gain;
	float   delay[MAX_ECHOS], decay[MAX_ECHOS];
	int samples[MAX_ECHOS], maxsamples;
	//size_t fade_out;
};


struct priv_t *echo;
/*
 * play file.wav 0.8 0.88 60 0.4 
 * 0.8 in-gain
 * 0.88 out_gain
 * 60 delay
 * 0.4 decay
 * /
/**< The sample rate. (e.g. 44100) */

int echo_init(void)
{
	int i;

	int rate = 48000;
	/**< The sample rate. (e.g. 44100) */


	echo = (struct priv_t*)malloc(sizeof(struct priv_t));
	echo->counter = 0;
	echo->num_delays = 1;
	echo->in_gain = 0.8; //between 0 to 1
	echo->out_gain = 0.88; //between 0 and 1
	echo->delay[0] = 60; //ms
	echo->maxsamples = 0;
	echo->decay[0] = 0.4; //衰减值

	for (i = 0; i < echo->num_delays; i++)
	{
		echo->samples[i] = echo->delay[i] * rate /1000;

		if(echo->samples[i] < 1)
		{
			return -1;
		}

		if(echo->samples[i] > DELAY_BUFSIZ)
		{
			return -2;
		}

		if(echo->decay[i] < 0.0)
		{
			return -3;
		}

		if(echo->decay[i] > 1.0)
		{
			return -4;
		}

		if(echo->samples[i] > echo->maxsamples )
		{
			echo->maxsamples = echo->samples[i];
		}
	}

	echo->delay_buf = (double*)malloc(sizeof(double) * echo->maxsamples);

	for(int j = 0; j < echo->maxsamples; ++j)
	{
		echo->delay_buf[j] = 0;

	}
	return 0;
}

void echo_process(short* signal_in, short* signal_out, int in_length)
{
	int d_in, d_out;

	while(in_length--)
	{
		d_in = *signal_in++;

		d_out = d_in * echo->in_gain;

		for(int j = 0; j < echo->num_delays; j++)
		{
			d_out += echo->delay_buf[(echo->counter + echo->maxsamples - echo->samples[j]) % echo->maxsamples] * echo->decay[j];
		}

		d_out = d_out * echo->out_gain;

		*signal_out++ = d_out;

		echo->delay_buf[echo->counter] = d_in;

		echo->counter = (echo->counter + 1) % echo->maxsamples;
	}
}

void exit_echo(void)
{
	free(echo);
}


void resample_8k_up16k(short* pInAudioData, int nInAudioLen, short* pOutAudioData, int& nOutAudioLen)
{
	short* sSampleIn = pInAudioData;
	int nFrequency = 0;
	while (sSampleIn - pInAudioData < nInAudioLen)
	{
		memcpy((char*)(pOutAudioData + nFrequency), (char*)sSampleIn, 2);
		nFrequency++;
		memcpy((char*)(pOutAudioData + nFrequency), (char*)sSampleIn, 2);
		nFrequency++;
		sSampleIn++;
	}

	nOutAudioLen = nFrequency*2;
}
