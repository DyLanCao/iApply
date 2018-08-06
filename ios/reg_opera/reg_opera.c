#include <stdio.h>
unsigned int reg1 = 0xFFFF;

int main()
{

    unsigned int reg2 = 0;

    reg1^= 0x1 << 4;
   
    printf("0x%x \n\t",reg1);

return 0;
}
