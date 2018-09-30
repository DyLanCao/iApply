#include <stdio.h>

int main()
{

	short* __restrict__ ptest = (short*)malloc(16*sizeof(short));
	char* __restrict__ ctest = (char*)malloc(32*sizeof(char));

	for(int icnt =0 ; icnt < 32; icnt++)
	{
		ctest[icnt] = 0x55;
	}

	memcpy((unsigned int)ptest + 0,(unsigned int)ctest, 16);
	
	for(int cnt =0 ; cnt < 8; cnt++)
	{
		printf("cnt:%d data:%d \n",cnt,(unsigned int)ptest[cnt]);
	}
	
	printf("size:%d",sizeof(unsigned int));

	free(ptest);
	free(ctest);

	return 0;
}
