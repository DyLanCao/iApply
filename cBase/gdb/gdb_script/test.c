#include <stdio.h>

int global = 0;

int fun_1(void)
{
    return 1;
}

int fun_a(void)
{
    int a = 0;
    printf("%d\n", a);
}

int main(void)
{
    fun_a();
    return 0;
}
