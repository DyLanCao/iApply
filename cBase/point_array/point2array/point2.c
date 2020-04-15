#include <stdio.h>

//static int array[2];
int *array= NULL;
int main()
{

	int *ptest = NULL;

	ptest = (int*)malloc(2*sizeof(int));

	ptest[0] = 32767;
	ptest[1] = -32767;

	array = ptest;
	printf("val1:%d val2:%d \n",array[0],array[1]);

	return 0;

}
