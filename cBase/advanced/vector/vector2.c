#include <stdio.h>

typedef int v4si __attribute__ ((vector_size (16)));

int main()
{
	v4si a = {1,2,3,4};
	v4si b = {5,6,7,8};
	v4si mask1 = {0,1,1,3};
	v4si mask2 = {0,4,2,5};
	v4si res;
	v4si res1;

	res = __builtin_shuffle (a, mask1);       /* res is {1,2,2,4}  */
	res1 = __builtin_shuffle (a, b, mask2);    /* res is {1,5,3,6}  */


	for(int cnt =0; cnt < 4; cnt++)
	{
	
		printf("%d \n",res[cnt]);
		printf("%d \n",res1[cnt]);
	}

	return 0;
}
