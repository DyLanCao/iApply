#include<stdio.h>
#include<stdlib.h>
void main()
{
int *a;
int buff_alloced = 0;

while(1)
{
 a=(int *)malloc(1000*sizeof(int));
 buff_alloced +=1000*sizeof(int);
 printf("buff_sized:%d \n\t ",buff_alloced);
 }
}
