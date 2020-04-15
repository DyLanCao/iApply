#include <stdio.h>
#include <string.h>


int main()
{
	unsigned char test1_arr[32] = "hello world";
	unsigned char test2_arr[32] = "hello world";

	int ret = memcmp(test1_arr,test2_arr,strlen(test1_arr));
	printf("unsigned char memcmp is:%d \n\t ",ret);

	char test3_arr[32] = "hello world";
	char test4_arr[32] = "hello world";

	int reta = strcmp(test3_arr,test4_arr);
	printf("char strcmp is:%d \n\t ",reta);

	unsigned char test5_arr[32] = "hello world";
	unsigned char test6_arr[32] = "hello world";

	int retb = strcmp(test5_arr,test6_arr);
	printf("char strcmp is:%d \n\t ",retb);

	return 0;
}
