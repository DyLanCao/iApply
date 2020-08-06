#include <stdio.h>

void SVC_Handler (void) __attribute__((weak));

#pragma weak func2


void func2(void)
{
    printf("func2 is test\n");
}
void SVC_Handler (void) __attribute__((weak));

void SVC_Handler (void)
{
    int cnt = 0;


    while(1)
    {
        sleep(1);
        printf("svc handler cnt:%d \n",cnt++);
    }
}

#define vPortSVCHandler     SVC_Handler

int main()
{

    func2();
    vPortSVCHandler();


    return 0;
}

