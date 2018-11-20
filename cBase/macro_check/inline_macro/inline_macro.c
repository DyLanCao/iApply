#include <stdio.h>
/* Position of the most significant bit of x */
#define gap8_fl1(x)				(31 - __builtin_clz((x)))

static int array[32];
int main()
{
	int test = 0xEFFFFFFF;
	int cnt = 0;

	while(cnt <10)
	{
		printf("test:%x gap8:%d \n",test,gap8_fl1(test));
		test = test>>1;
		cnt++;
	}

	printf("test:%x gap8:%d \n",0,gap8_fl1(0));
	printf("test:%x gap8:%d \n",1,gap8_fl1(1));

	return 0;
}
