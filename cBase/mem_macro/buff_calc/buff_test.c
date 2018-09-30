#include <stdio.h>
char buf[16];
typedef unsigned char uint8_t;

int main()
{
   
    for(int cnt=0; cnt < 16; cnt += 8)
    {
        for(int ict = 0; ict < 8; ict++)
        {
            buf[cnt + ict] = ict;
            printf("%d \n\t",buf[cnt + ict]);
        }
    }

}
