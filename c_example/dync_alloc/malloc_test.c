#include <stdio.h>

int main()
{

	int *ptest = (int)malloc(16*sizeof(int));
	printf("addr:0x%x",&ptest);

	free(ptest);

	return 0;
}
