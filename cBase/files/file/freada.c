#include <stdio.h>
#include <string.h>


#define BUFFSIZE 1024

int main(int argc, char **argv){

    char buff[BUFFSIZE];
    //FILE *fd = fopen (argv[1], "rb");
    int count, errno=0;

    bzero (buff, BUFFSIZE);
    while (1){
        count = fread (buff, sizeof (char), BUFFSIZE, stdin);
        //int n = feof (fd);
        printf ("%d,%d\n", count);
        //printf ("%s\n",strerror (errno));
    }
    return 0;
}
