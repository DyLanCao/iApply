#include <stdio.h>

int array[10] = {-23160,32766,23197,52,-23123,-32766,-23234,-108};

short btest[10];

int main()
{
	int i = 9;

	for(int i =0; i < 10; i++)
	{
		//btest[i] = (short)array[i];
		printf("%d \n",(short)array[i]);
	}

	printf("size: %d \n",sizeof(short int));
	printf("size: %d \n",sizeof(short));
	return 0;
}
