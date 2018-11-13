#include <stdio.h>

typedef int v4si __attribute__ ((vector_size (16)));

int main()
{
	v4si a, b, c;
	long l;

	a = b + 1;    /* a = b + {1,1,1,1}; */
	a = 2 * b;    /* a = {2,2,2,2} * b; */

	a = l + a;    /* Error, cannot convert long to int. */
	printf("%d",a);

	return 0;
}
