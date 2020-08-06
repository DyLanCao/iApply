#include <stdio.h>

int main()
{
    char ch = 'a';
    char *cp = &ch;

    printf("*cp is:%c \n",++*cp);
    printf("*(cp + 1) is:%c \n",*(cp + 1));
    printf("cp is:%s \n",cp);
    printf("*(cp + 2) is:%c \n",*(cp + 2));

    return 0;
}
