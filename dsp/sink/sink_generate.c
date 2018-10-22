#include <stdio.h>
#include <stdlib.h>

#define pi 3.14

void sink_generate()
{
	double f = 1;
	double fs = 48000;
	double db = -10.0f;
	double duration = 1;
	double incr = 2 * pi * f / fs ;// 數字頻率，也是相鄰兩個採樣點的變化的弧度
	//double A = powf(10,db / 20); // 波形的最大幅度值
	double A = 32767;// 波形的最大幅度值
	float* frame = (float*)malloc(sizeof(float)*duration*fs);
	printf("%d",(int)(duration*fs));

	
	for(int i=0; i < (int)(duration*fs); i++)
	{
	      frame[i] = A*sin(i * incr);
		  if((i % 6000) == 0)
		  {	
			  printf("%d\n",(int)frame[i]);
		  }
	}
	

    printf("%d\n",(int)frame[(int)(duration*fs) - 1]);

	free(frame);
}

int main()
{
	sink_generate();

	return 0;
}