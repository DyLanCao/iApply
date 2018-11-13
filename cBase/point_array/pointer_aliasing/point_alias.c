#include <stdio.h>

int foo(int *a, int *b)
{
	*a = 5;
	*b = 6;
	return *a + *b;
}

int foo_restrict(int *restrict a, int *restrict b)
{
	*a = 5;
	*b = 6;
	return *a + *b;
}
int rfoo(int *restrict a, int *restrict b)
{
    *a = 5;
    *b = 6;
    return *a + *b;
}
int main()
{

	int i = 0;
	int *a = &i;
	int *b = &i;

	//printf("foo:%d\n",foo(a,b));
	printf("foo_res:%d\n",foo_restrict(a,b));
	printf("foo_res:%d\n",rfoo(a,b));

	return 0;
}
