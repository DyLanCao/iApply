#include <stdio.h>

#define DUMBCOPY for(i = 0; i < 65535; i++) \
	destination[i] = source[i]

#define SMARTCOPY memcpy(destination, source,65536)

int main()
{
	char source[65536], destination[65536];

	int i,j;

	for(j = 0; j < 100000; j++)
	{
		//SMARTCOPY;
		DUMBCOPY;
	}

	return 0;
}
