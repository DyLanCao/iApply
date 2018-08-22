#include <stdio.h>

int main()
{
	int cnt = (int)((16+sizeof(int) - 1)/sizeof(int));

	printf("cnt:%d",cnt);

	return 0;
}
