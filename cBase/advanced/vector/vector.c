#include <stdio.h>

typedef int v4si __attribute__ ((vector_size (16)));

int main()
{
	v4si a, b, c;
	long l;
	
	b = (v4si){2,3,4,5};
	a = b + 1;    /* a = b + {1,1,1,1}; */
	//a = 2 * b;    /* a = {2,2,2,2} * b; */

	//a = 1 + a;    /* Error, cannot convert long to int. */
	for(int cnt =0; cnt < 4; cnt++)
	{
	
		printf("%d \n",a[cnt]);
	//	printf("%d,%d \n",a[cnt][0],a[cnt][1]);
	}

	return 0;
}
