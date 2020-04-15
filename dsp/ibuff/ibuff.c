#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DELAY_BUFSIZ ( 50 * 50U * 1024 )
#define MAX_ECHOS 7     /* 24 bit x ( 1 + MAX_ECHOS ) = */
/* 24 bit x 8 = 32 bit !!!      */
typedef unsigned char  U8;
typedef unsigned int uint32_t;

const unsigned char CN_POWER_ON [] = {
//#include "SAM16K.txt"
#include "SOUND_ANSWER.txt"
};

typedef enum
{
	AUD_ID_POWER_ON = 0x0,
	AUD_ID_POWER_OFF,
	AUD_ID_LANGUAGE_SWITCH,
	MAX_RECORD_NUM
}AUD_ID_ENUM;
/**< The sample rate. (e.g. 44100) */
static short *g_app_audio_data = NULL;
static uint32_t g_app_audio_length = 0;

int ibuff_init(void)
{
	int aud_id = 0x0;
	

	switch(aud_id)
	{
		case AUD_ID_POWER_ON:   
			g_app_audio_data = (short*)CN_POWER_ON; //aud_get_reouce((AUD_ID_ENUM)id, &g_app_audio_length, &type);
			g_app_audio_length = sizeof(CN_POWER_ON);
			break;
		default:
			g_app_audio_length = 0;
			break;
			int rate = 48000;
			/**< The sample rate. (e.g. 44100) */
	}

	g_app_audio_data = (short*)malloc(g_app_audio_length*sizeof(short));
	memcpy(g_app_audio_data,CN_POWER_ON,2*g_app_audio_length);
	printf("g_app_audio_length:%d ",g_app_audio_length);
	return 0;
}

void ibuff_process(short* signal_in, short* signal_out, int in_length)
{
	for(int icnt = 0; icnt < 100; icnt++)
	{
		printf("%5d ",g_app_audio_data[icnt]);
	}
}

void exit_ibuff(void)
{
	free(g_app_audio_data);
}
