#include <stdio.h>
char ibuff[32] = "hello world";

char *test(void)
{
        //static char ibuff[32];
        memcpy(ibuff,"new",12);

        return ibuff;
}
int main()
{
        printf("hello world:%s\n\t",test());

        return 0;
}
