#include <stdio.h>

int getSum(int num1,int num2);

int main(int argc, char *argv[])
{
	int num1 = 20;
	int num2 = 30;
	int sum = 0;

	num1 = num1 - 10;

	sum = getSum(num1,num2);

	printf("The sum is %d\n", sum);

	return 1;

}

int getSum(int num1,int num2)
{
	
	printf("The numbers are %d and %d\n", num1,num2);

	return (num1 + num2);
}
