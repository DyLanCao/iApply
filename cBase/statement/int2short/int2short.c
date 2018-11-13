#include <stdio.h>
#include <string.h>
#include <assert.h>
 

typedef short uint16_t;
typedef int uint32_t;

void audio_mono2stereo_16bits(uint16_t *dst_buf, uint16_t *src_buf, uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; ++i) {
        dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i]>>1;
    }
}

void audio_stereo2mono_16bits(unsigned char channel, uint16_t *dst_buf, uint16_t *src_buf, uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; i+=2) {
        dst_buf[i/2] = src_buf[i + channel];
    }
}

void audio_stereo2mono_16bits_check(unsigned char channel, int *dst_buf, int *src_buf, uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; i+=2) {
        dst_buf[i/2] = src_buf[i + channel];
    }
}

#define BUFF_SIZEA 160

static short iarray[BUFF_SIZEA];
static short oarray[2*BUFF_SIZEA];

void dump16(short *ibuf, int length)
{
  int j = 0;

  for (j=0; j<length; j++) {
   //if (!(j%20)) printf("%d == ",j);
   printf(" %d, ", ibuf[j]);
   if (!((j+1)%10)) printf("\n");
 }
}
int main()
{
        int ibuff[160],outbuf[160];
        int ilen = BUFF_SIZEA;
        for(int icnt = 0; icnt < BUFF_SIZEA; icnt++)
        {
                iarray[icnt] = -icnt;
        }
 
	audio_stereo2mono_16bits(0,oarray,iarray,BUFF_SIZEA);
	printf("input data.........................................\n");
	dump16(iarray,30);
	printf("output data.........................................\n");
	dump16(oarray,30);
	audio_stereo2mono_16bits_check(0,oarray,iarray,BUFF_SIZEA);
	//audio_mono2stereo_16bits(oarray,iarray,30);
	printf("output data2.........................................\n");
	dump16(oarray,30);

        return 0;
}
