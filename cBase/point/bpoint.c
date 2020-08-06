#include <stdio.h>

int strlen(char *string)
{
    int length = 0;


    while(*string++ != '\0')
        length += 1;


    return length;
}
int main()
{
    char test[] = "abcdefds";

    int len = strlen(test);
    printf("len is %d \n",len);

    return 0;
}
