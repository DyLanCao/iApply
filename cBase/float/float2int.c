

#include <stdio.h>

int main()
{
        
    double test = -12.1234;

    int a_int = (int)test;

    int b_int = (test - a_int)*10000;

    printf("a_int:%d b_int:%d ",a_int,b_int);


    return 0;
}
