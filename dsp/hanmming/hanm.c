#include <stdio.h>
#include <math.h>

#define PI 3.14
#define HANM_LEN 128

float hanm[HANM_LEN];

void hanm_func(void)
{
   float a = 0.46;
   
   for(int icnt = 0; icnt < HANM_LEN; icnt++)
   {
	hanm[icnt] = (1 -a) - a*cos(2*PI*icnt/(HANM_LEN -1));
   }
   for(int icnt = 0; icnt < HANM_LEN; icnt++)
   {
	printf("icnt:%d  val:%f \n",icnt,1000*hanm[icnt]);
   }
}
int main()
{

	hanm_func();

	return 0;
}


