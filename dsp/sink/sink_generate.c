#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define pi 3.14

int main(int argc, char** argv)
{

	FILE *Ifp,*ttt;

	if(argc != 2)
	{   
		printf("usage:./gwav XX.wav \n\t");
		return 0;
	}   

	Ifp = fopen(argv[1],"w");

	double f = 1;
	double fs = 48000;
	double db = -10.0f;
	double duration = 1;
	double incr = 2 * pi * f / fs ;// 數字頻率，也是相鄰兩個採樣點的變化的弧度
	//double A = powf(10,db / 20); // 波形的最大幅度值
	short A = 1000;// 波形的最大幅度值
	short* frame = (short*)malloc(sizeof(short)*duration*fs);
	//printf("%d",(int)(duration*fs));


	for(int i=0; i < (int)(duration*fs); i++)
	{
		frame[i] = A*sin(i * incr);
	
	}

	fwrite(frame,sizeof(short),duration*fs,Ifp);

	printf("numbers: %d\n",(int)frame[(int)(duration*fs) - 1]);
	free(frame);
	fclose(Ifp);

	return 0;
}
