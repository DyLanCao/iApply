#include <stdio.h>
#include <string.h>
#include <assert.h>
 
#define BUFF_SIZE 2048
 
static short iarray[BUFF_SIZE];
static short oarray[2*BUFF_SIZE];
 
unsigned int ibuff_used = 0;
 
unsigned int buff_alloc_free()
{
 
        return BUFF_SIZE - ibuff_used;
}
 
void buff_alloc_check(unsigned char **buff, unsigned int len)
{
        if(len > buff_alloc_free())
        {
        printf("alloc fialed len:%d freed:%d\n\t",len,buff_alloc_free());
        assert(len > buff_alloc_free());
        }
 
        *buff = iarray + ibuff_used;
        ibuff_used += len;
}
void HexDump(char *buf,int len,int addr)
{
    int i,j,k;
    char binstr[80];
 
    for (i=0;i<len;i++) {
        if (0==(i%16)) {
            sprintf(binstr,"%08x -",i+addr);
            sprintf(binstr,"%s %02x",binstr,(unsigned char)buf[i]);
        } else if (15==(i%16)) {
            sprintf(binstr,"%s %02x",binstr,(unsigned char)buf[i]);
            sprintf(binstr,"%s  ",binstr);
            for (j=i-15;j<=i;j++) {
                sprintf(binstr,"%s%c",binstr,('!'<buf[j]&&buf[j]<='~')?buf[j]:'.');
            }
            printf("%s\n",binstr);
        } else {
            sprintf(binstr,"%s %02x",binstr,(unsigned char)buf[i]);
        }
    }
    if (0!=(i%16)) {
        k=16-(i%16);
        for (j=0;j<k;j++) {
            sprintf(binstr,"%s   ",binstr);
        }
        sprintf(binstr,"%s  ",binstr);
        k=16-k;
        for (j=i-k;j<i;j++) {
            sprintf(binstr,"%s%c",binstr,('!'<buf[j]&&buf[j]<='~')?buf[j]:'.');
        }
        printf("%s\n",binstr);
    }
}
/*
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
*/

typedef short uint16_t;
typedef int uint32_t;

void audio_mono2stereo_16bits(uint16_t *dst_buf, uint16_t *src_buf, uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; ++i) {
        dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i]>>1;
    }
}
 
int main()
{
        unsigned char *ibuff = NULL,*outbuf = NULL;
        int ilen = BUFF_SIZE - 1;
        for(int icnt = 0; icnt < BUFF_SIZE/4; icnt++)
        {
                iarray[icnt] = -icnt;
        }
 
       // buff_alloc_check(&ibuff,ilen);
        buff_alloc_check(&outbuf,2*ilen);
	
	
	audio_mono2stereo_16bits(oarray,iarray,30);
	
	for(int icnt = 0; icnt < 30; icnt++)
	{
		printf("oarray:%d ",oarray[icnt]);
	}
        //HexDump(ibuff,ilen,(int)ibuff);
        //HexDump(outbuf,2*ilen,(int)outbuf);
 
        return 0;
}
