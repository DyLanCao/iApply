#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(){
    char *s = "ssid:vision";
    char *p;
    p = strrchr(s, 'ssid:');
    printf("%s\n", s);
    printf("%s\n", p);
    system("pause");
    return 0;
}
