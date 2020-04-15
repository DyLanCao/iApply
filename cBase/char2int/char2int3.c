
#include <stdio.h>
typedef unsigned char uint8_t;
int parseInt(uint8_t* chars, int len)
{
    int sum = 0;
    //int len = strlen(chars);
    for (int x = 0; x < len; x++)
    {
        int n = chars[len - (x + 1)] - '0';
        sum = sum + powInt(n, x);
    }
    return sum;
}

int powInt(int x, int y)
{
    for (int i = 0; i < y; i++)
    {
        x *= 16;
    }
    return x;
}

int main()
{

        uint8_t* something = "12";

        unsigned short number = (unsigned short)something;
        printf("number is:%d ",number);

        return 0;
}
