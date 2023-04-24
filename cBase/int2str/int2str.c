#include <stdio.h>

#ifndef ABS
#define ABS(x)                          ((x<0)?(-(x)):(x))
#endif

int itoa(int i,char* string) 
{
    int power, j,icnt = 0; 
    j=i; 
    for(power=1;j>=10;j/=10)
        power*=10; 
    for(;power>0;power/=10) 
    { 
        *string++='0'+i/power; 
	icnt++;
        i%=power; 
    }

    *string='\0'; 

    return icnt;
}

int short2str(short *ival,int length,char* string) 
{
    int power, j,icnt = 0;

    for(int iss = 0; iss < length;iss++)
    {

    	j=ival[iss]; 

	if(j < 0)
	{

        	*string++='-'; 
		icnt++;
		j = ABS(ival[iss]);
		ival[iss] = -ival[iss];
	}

    	for(power=1;j>=10;j/=10)
	{

        	power*=10; 
	}

    	for(;power>0;power/=10) 
    	{ 
        	*string++='0'+ival[iss]/power; 
		icnt++;
        	ival[iss]%=power; 
    	}

    	*string++=','; 
	icnt++;
    }


    return icnt;
}

char test[16];
char testa[160];

int main()
{
	int val = 10000;
	int ret = itoa(val,test);
	printf("ret is:%d test is:%s",ret,test);

	short ival[4] = {1000,200,100,10};

	ret = short2str(ival,4,testa);
	printf("ret is:%d testa is:%s",ret,testa);
	return ret;
}


