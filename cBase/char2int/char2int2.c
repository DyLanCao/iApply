
#include <stdio.h>

int main()
{
  char test[] = {'1','2'};

 int num = 10*((int)test[0] - (int)'0') + (int)test[1] - (int)'0';

 printf("hello world:%c",test[0]);
 printf("number is:%d ",num);

 return 0;
}
