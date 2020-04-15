#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
  
int main()  
{  
    int number, i;  
    char str[10];  
  
    while(scanf("%d", &number) != EOF)  
    {  
        memset(str, 0, sizeof(str));  
      
        i = 0;  
        while(number)  
        {  
            str[i ++] = number % 10 + '0';  
            number /= 10;  
        }         
        puts(str);        
    }  
  
    return 0;  
}
