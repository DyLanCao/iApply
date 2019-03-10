#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define W 128

int FileSet = 0;
int FileEnd = 0;
int FileLength = 0;
short InputData[W];
/*
void buf_txt(short *buf,int len,FILE*ttt)
{
	for(int i=0; i < len; i++)
	{
		fprintf(ttt, ",%d",buf[i]);
		if(i % 64 == 0)
		{
//fprintf(ttt, "\n");
		}
	}
}
*/
int main(int argc, char** argv)
{

	FILE *Ifp,*ttt;
	
	if(argc != 2)
	{
		printf("usage:./gwav XX.wav \n\t");
		return 0;
	}

	Ifp = fopen(argv[1],"w");

	//ttt = fopen(argv[2],"w");
	/*
	fseek(Ifp,0L,SEEK_END);
	FileEnd=ftell(Ifp);
	rewind(Ifp);
	FileLength = FileEnd/2;
	*/
	int jcnt = 0;

	while(jcnt++ < 100)
	{
		for(int icnt = 0; icnt < W; icnt++)
		{
			InputData[icnt] = rand() % 255 - 128;
			printf("%d, ",InputData[icnt]);
		}

		fwrite(InputData,sizeof(short),W,Ifp);
	}

	//fread(InputData,sizeof(short),FileLength,Ifp);

	//buf_txt(InputData,FileLength,ttt);
	fclose(Ifp);

	return 0;
}
