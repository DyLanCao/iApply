#include <stdio.h>
short sinka[] = {-32767, -20886, 6139, 28713, 30465,10125,-17557, -32508};
short sinkb[64] = {};
#define SINK_LEN 64
short sink_in[64] = {};
short sink_out[64] = {};

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

for(int cnt = 0; cnt < 64; cnt++)
{
	if((cnt + 1)%8 == 0)
	{
		printf("%d \n",sink_in[cnt]);
	}
	else 
	{
		printf("%d, ",sink_in[cnt]);
	}
}

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

return 0;
}
