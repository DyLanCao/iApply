#include <stdio.h>
#include <string.h>

short sinka[] = {-32767, -20886, 6139, 28713, 30465,10125,-17557, -32508};
short sinkb[64] = {};
#define SINK_LEN 64
short sink_in[64] = {};
short sink_out[64] = {};
unsigned char buff[] = {01,01011011110111111111111111111111011111101101101010100100100000010000000000000000000001000010010101};
void pcm_convert_pdm(short *inpcm,short *outpdm)
{
	int qe = 0;

	for(int cnt = 0; cnt < 64; cnt++)
	{
		if(inpcm[cnt] >= qe)
		{
			outpdm[cnt] = 1;
		}
		else
		{
			outpdm[cnt] = -1;
		}

		qe = outpdm[cnt] - inpcm[cnt] + qe;
	}

}
int main()
{
for(int cnt = 0; cnt < 64; cnt += 8)
{
	memcpy(sink_in + cnt,sinka,8*sizeof(short));

}

pcm_convert_pdm(sink_in,sink_out);

#if 1
for(int cnt = 0; cnt < 64; cnt++)
{
	if((cnt + 1)%8 == 0)
	{
		printf("%d \n",sink_out[cnt]);
	}
	else 
	{
		printf("%d, ",sink_out[cnt]);
	}
}

#endif

#if 0
for(int cnt = 0; cnt < 32; cnt++)
{
	if((cnt + 1)%8 == 0)
	{
		printf("%d \n",buff[cnt]);
	}
	else 
	{
		printf("%d, ",buff[cnt]);
	}
}
#endif
return 0;
}
