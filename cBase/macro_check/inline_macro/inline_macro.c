#include <stdio.h>
#include <time.h>

#define NUM(x) x*x*x

static inline int num_test(int x)
{
	return x*x*x;
}

static int array[32];
int main()
{
	int test = 0xEFFFFFFF;
	int cnt = 0;
	long long num = 0,num1 = 0;
    clock_t start, finish;
	double duration;  

	start = clock(); 
	while(cnt++ < 10000)
	{
		num += NUM(cnt);	
	
	}
	finish = clock(); 

	duration = (double)(finish - start) / CLOCKS_PER_SEC;  
	printf( "%f seconds \n\t", 1000*duration ); 

	cnt = 0;
	start = 0, finish = 0;

	start = clock(); 
	while(cnt++ < 10000)
	{
		num1 += num_test(cnt);
	}

	finish = clock(); 

	duration = (double)(finish - start) / CLOCKS_PER_SEC;  
	printf( "%f ms seconds \n\t", 1000*duration ); 

	printf("num:%d num1:%d \n\t",num, num1);

	return 0;
}
