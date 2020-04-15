#include <stdio.h>

//short array[10] = {-23160,32766,23197,52,-23123,-32766,-23234,-108};
int array[10] = {0xFFFFFFFF,0xFFFFFFF4,0xFFFFFFF7,0xF0000001,-2,-32766,-23234,-108};

int btest[10];

int main()
{
	int i = 9;

	for(int i =0; i < 10; i++)
	{
		printf("%d \n",array[i]);
	}

	printf("size: %d \n",sizeof(short int));
	printf("size: %d \n",sizeof(short));
	return 0;
}
