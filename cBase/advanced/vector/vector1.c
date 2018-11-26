#include <stdio.h>

typedef int v4si __attribute__ ((vector_size (16)));

int main()
{
	v4si a = {1,2,3,4};
	v4si b = {3,2,1,4};
	v4si c;

	//c = a >  b;     /* The result would be {0, 0,-1, 0}  */
	c = a == b;     /* The result would be {0,-1, 0,-1}  */

	//a = 1 + a;    /* Error, cannot convert long to int. */
	for(int cnt =0; cnt < 4; cnt++)
	{
	
		printf("%d \n",c[cnt]);
	//	printf("%d,%d \n",a[cnt][0],a[cnt][1]);
	}

	return 0;
}
