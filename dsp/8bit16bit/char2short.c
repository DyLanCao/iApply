#include <stdio.h>


const unsigned char CN_POWER_ON [] = {
#include "SOUND_ANSWER.txt"
};

short test_array[200];

void char2short(unsigned char* pchar, unsigned short* pshort)
{
  *pshort = (pchar[0] << 8) | pchar[1];
}

int main()
{
  unsigned char test[2];
  unsigned short result = 0;

  for(int cnt = 0; cnt < 100; cnt +=2)
  {

  	char2short(CN_POWER_ON + cnt, &result);
  	printf(" %d\n",result);
  }

  return 0;
}
