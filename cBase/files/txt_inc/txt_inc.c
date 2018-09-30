#include <stdio.h>


const short CN_POWER_ON [] = {
#include "SINK1.txt"
};

int g_app_audio_length = 0;

int main()
{
	g_app_audio_length = sizeof(CN_POWER_ON);
	printf("size of:%d",g_app_audio_length);
	return 0;
}
