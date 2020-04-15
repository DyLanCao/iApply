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
		//fprintf(ttt, "\n");
		
		if(i % 32 == 0)
		{
			fprintf(ttt, "\n");
		}
		
	}
}

int main(int argc, char** argv)
{

	FILE *Ifp,*ttt;
	
	if(argc != 3)
	{
		printf("usage:./wav2txt XX.wav EEE.txt\n\t");
		return 0;
	}

	Ifp = fopen(argv[1],"rb");

	ttt = fopen(argv[2],"w");

	fseek(Ifp,0L,SEEK_END);
	FileEnd=ftell(Ifp);
	rewind(Ifp);
	FileLength = FileEnd/2;

	while(FileLength >= W)
	{
		fread(InputData,sizeof(short),W,Ifp);
		buf_txt(InputData,W,ttt);
		FileLength -= W;
	
	}

	fread(InputData,sizeof(short),FileLength,Ifp);

	buf_txt(InputData,FileLength,ttt);

	return 0;
}
