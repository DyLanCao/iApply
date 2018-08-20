#include <stdio.h>
typedef struct
{
	int testa;
	int testb;
}str_test;

str_test* g_str_test;

typedef struct myAutoBuffer
{
        //enum { buffer_padding = (int)((16+sizeof(int) - 1)/sizeof(int)) };
        enum { buffer_padding = (int)((16+sizeof(int) - 1)/sizeof(int)) } buffer_pad;
        void* ptr;
        size_t size;
}AB_ ;

//enum { buffer_padding = (int)((16+sizeof(int) - 1)/sizeof(int)) } buffer_pad;

int main()
{

	AB_ str_test;

	//g_str_test->testa = 10;
	//printf("testa:%d \n\t",g_str_test->testa);
	//printf("testb:%d \n\t",g_str_test->testb);
	str_test.buffer_pad = buffer_padding;

	printf("buffer_padding:%d \n\t",str_test.buffer_pad);

return 0;
}
