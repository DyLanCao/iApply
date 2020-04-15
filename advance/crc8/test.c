#include <stdio.h>
#include <string.h>
#include "secure.h"

unsigned char tsta[8] = {1,2,3,4};
unsigned char tstb[8] = {'a','b','c','d'};
typedef unsigned char uint8_t;

int main()
{
    //unsigned char tsta[4] = {1,2,3,4};
    
    //uint8_t testa = malloc(20);
    uint8_t data[8] = {0xBE,0xEF,0,0,0,0,0,0};


    unsigned char crc1,crc2;
    printf("tsta len:%d tstb len:%d \n\t",sizeof(tsta),sizeof(tstb));
    printf("char len:%d \n\t",sizeof(unsigned char));
    crc1 = crc8(tsta, 4);
    //crc2 = crc8(testa, 1);
    //crc1 = Crc8_cal(testa, 20);
    //free(testa);
    printf("crc1:%d crc2:%x \n\t",crc1,crc2);


    return 0;
        
        
}
