#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#if 1
unsigned __int64 GetCycleCount()
{
    __asm
    {
        _emit 0x0F;
        _emit 0x31;
    }
}
#endif

int main()
{
    unsigned long t1,t2;
    t1 = (unsigned long)GetCycleCount();
    t1sd = aGetTickCount();
   while(int cnt++ < 1000000000);

    t2 = (unsigned long)GetCycleCount();
    printf("Use Time:%f\n",(t2 - t1)*1.0/FREQUENCY);   //FREQUENCY指CPU的频率
}
