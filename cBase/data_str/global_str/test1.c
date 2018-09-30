#include <stdio.h>
typedef struct
{
	int testa;
	int testb;
}str_test;

str_test* g_str_test;

int main()
{
	g_str_test->testa = 10;
	printf("testa:%d \n\t",g_str_test->testa);
	printf("testb:%d \n\t",g_str_test->testb);
}
