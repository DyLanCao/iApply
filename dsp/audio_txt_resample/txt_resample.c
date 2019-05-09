#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define W 8
typedef unsigned char U8;

int FileSet = 0;
int FileEnd = 0;
int FileLength = 0;
//U8 InputData[W];
void buf_txt(short *buf,int len,FILE*ttt)
{
	for(int i=0; i < len; i++)
	{
		//fprintf(ttt, ",%d",buf[i]);
		//if(i % 2 == 0)
		{
			fprintf(ttt, ",%d",buf[i]);
		}
	}
}
const U8 InputData[] = { 
#include "qcy/anc_on.txt"
};
int main(int argc, char** argv)
{

	FILE *ttt;
	

	//Ifp = fopen(argv[1],"rb");

	ttt = fopen(argv[1],"w");

	FileLength =sizeof(InputData)/2;
	printf("file length:%d ",FileLength);

	while(FileLength >= W)
	{
		buf_txt((short*)InputData,W,ttt);
		FileLength -= W;
		printf("count:%d ",FileLength);
	}

//fread(InputData,sizeof(U8),FileLength,Ifp);

//buf_txt(InputData,FileLength,ttt);

	return 0;
}
