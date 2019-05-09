#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define W 128

int FileSet = 0;
int FileEnd = 0;
int FileLength = 0;
char InputData[W];
void buf_txt(short *buf,int len,FILE*ttt)
{
	for(int i=0; i < len; i++)
	{
		//fprintf(ttt, ",%d",buf[i]);
		if(i % 2 == 0)
		{
			fprintf(ttt, ",%d",buf[i]);
		}
	}
}

void bit8_to_bit16(char *Inputdata,int len,FILE*ttt)
{
	for(int i=0; i < len; i++)
	{
		short sData16 = (( short )( Inputdata[i] + 0x80 )) << 8;
		fprintf(ttt, ",%d",sData16);
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
		fread(InputData,sizeof(unsigned char),W,Ifp);
		bit8_to_bit16(InputData,W,ttt);
		#buf_txt(InputData,W,ttt);
		FileLength -= W;
	
	}

	fread(InputData,sizeof(short),FileLength,Ifp);

	buf_txt(InputData,FileLength,ttt);

	return 0;
}
