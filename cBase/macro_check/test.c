#include <stdio.h>
#include "checka.h"
#include "checkb.h"
#include <math.h>

int main()
{
	int testa,testb;
	testa = 1;
	testb = 3;
	testa = pow(testb,testa);
	testb = sin(10);
	printf("testa:%d testb:%d",testa,testb);

	return 0;
}

