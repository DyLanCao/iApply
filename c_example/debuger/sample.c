#include <stdio.h>
#include <stdlib.h>

int add(int x, int y)
{
	return x + y;
}

int main()
{
	int a  = 10, b = 20;
	int result;

	//fp = add;
	result = add(a, b);
	printf("%d \n\t",result);
}
