#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define W 128

int FileSet = 0;
int FileEnd = 0;
int FileLength = 0;
short InputData[W];
void buf_txt(short *buf,int len,FILE*ttt)
{
	for(int i=0; i < len; i++)
	{
		fprintf(ttt, ",%d",buf[i]);
		if(i % 64 == 0)
		{
			fprintf(ttt, "\n");
		}
	}
}

#define FILE_NUM 2

int main(int argc, char** argv)
{

	FILE *Ifp,*ttt;
	char file[100];
	
	for(int cnt =0 ; cnt < FILE_NUM; cnt++)
	{
		sprintf(file,"file_%d.txt", cnt);
		ttt = fopen(file,"w");
		
		for(int i = 0; i < 10; i++)
		{
			fprintf(ttt,"0x%x,",i+cnt);
		}
		fclose(ttt);
	}


	return 0;
}
