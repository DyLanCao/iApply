#include <stdio.h>

int min_fun(int a, int b, int c)
{
	    int min;
		return c<(min=a<b?a:b)?c:min;
}

int max_fun(int a, int b, int c)
{
	    int max;
		return c>(max=a>b?a:b)?c:max;
}

int main()
{
	int a =-5, b=6, c=7;

	int ret = min_fun(a,b,c);
	printf("min value:%d \n",ret);
	ret = max_fun(a,b,c);
	printf("max value:%d \n",ret);

	return 0;
}
