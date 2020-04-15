#include <stdio.h>

typedef unsigned char uint8_t;

uint8_t gencrc(uint8_t *data, size_t len)
{
    uint8_t crc = 0xff;
    size_t i, j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }
    return crc;
}

/*
crc8 poly = 0x107 (x8+x2+x1+1)
*/
uint8_t crc8(uint8_t *data, int size)
{
    uint8_t crc = 0x00;
    uint8_t poly = 0x07;
    int bit;

    while (size--)
    {
        crc ^= *data++;
        for (bit = 0; bit < 8; bit++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ poly;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

int main()
{
uint8_t data[8] = {0xBE,0xEF,0,0,0,0,0,0};
uint8_t datab[8] = {0xBE,0xEF,2,0,0,0,0,0};
uint8_t crc,crcb;
    crc = gencrc(data, 8);   
    crcb = gencrc(datab, 8);   
    printf("first crc:\n");
    printf("crc:0x%1x crcb:0x%x \n", crc,crcb);

    crc = crc8(data, 8);   
    crcb = crc8(datab, 8);   
    printf("second crc:\n");
    printf("crc:0x%1x crcb:0x%x \n", crc,crcb);

    crc = gencrc(data+2, 1); /* returns 0xac */
    printf("%1x\n", crc);
    return 0;
}
