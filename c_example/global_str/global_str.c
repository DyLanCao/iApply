#include <stdio.h>
typedef struct
{
	int testa;
	int testb;
}str_test;

str_test g_s_test;

int main()
{
	g_s_test.testa = 10;
	printf("testa:%d \n\t",g_s_test.testa);
	printf("testb:%d \n\t",g_s_test.testb);
}
