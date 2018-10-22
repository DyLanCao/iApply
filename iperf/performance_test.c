#include "stdio.h"

#include "time.h"
#include "string.h"

#define MEM_DEBUG
#define GLOBAL_NUM

#define rdtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#ifdef GLOBAL_NUM
static char array[1000];
#endif

typedef unsigned long long cycles_t;

unsigned long long get_cycles()
{
	unsigned low,high;
	unsigned long long val;

	rdtsc(low,high);
	val = high;
	val = (val << 32) | low;

	return val;
}

#ifdef MEM_DEBUG
void mem_alloc(void)
{
	int *buff = (int*)malloc(20000*sizeof(int));


	if(buff != NULL)
	{
		free(buff);
	}
}
#endif
int main()
{
	float mhz;
	//mhz = get_cpu_mhz();
	mhz = 780;
	cycles_t c1, c2;

	float tmp;

#ifdef MEM_DEBUG
	mem_alloc();
#endif

#ifdef GLOBAL_MEM
	for(int cnt = 0; cnt < 1000; cnt++)
	{
		array[cnt] = cnt % 255;
	}
#endif
	for(;;)
	{
		c1 = get_cycles();
		sleep(1);
		c2 = get_cycles();
		tmp = (float)(c2 - c1);
		printf("%f %lf , 1 sec = %f usec \n",tmp,mhz, tmp/mhz);
	}

	return 0;
}
