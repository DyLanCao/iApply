#include <stdio.h>

int fun_0(void)
{
	printf("%s: %d\n", __FUNCTION__, __LINE__);
	return 0;
}

int fun_1(void)
{
	printf("%s: %d\n", __FUNCTION__, __LINE__);
	return 0;
}

int fun_2(void)
{
	printf("%s: %d\n", __FUNCTION__, __LINE__);
	return 0;
}

int fun_3(void)
{
	printf("%s: %d\n", __FUNCTION__, __LINE__);
	return 0;
}

void main(void)
{
	int a = 0;
	int b = 12;
	fun_0();
	fun_3();
}
