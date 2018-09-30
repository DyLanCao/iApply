#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
typedef unsigned long long ULL;
#define BUFFER_SIZE ((int)(0x80 * sizeof(void*) * sizeof(ULL)))
 
#define sb_free(a)          ((a) ? free(stb__sbraw(a)), 0 : 0)
#define sb_push(a, v)       (stb__sbmaybegrow(a, 1), (a)[stb__sbn(a)++] = (v))
#define sb_count(a)         ((a) ? stb__sbn(a) : 0)
#define sb_add(a, n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a) - (n)])
#define sb_last(a)          ((a)[stb__sbn(a) - 1])
 
#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]
 
#define stb__sbneedgrow(a, n)  ((a) == 0 || stb__sbn(a) + (n) >= stb__sbm(a))
#define stb__sbmaybegrow(a, n) (stb__sbneedgrow(a, (n)) ? stb__sbgrow(a, n) : 0)
#define stb__sbgrow(a, n)      ((a) = stb__sbgrowf((a), (n), sizeof(*(a))))
 
static void * stb__sbgrowf(void *arr, int increment, int itemsize) {
 int dbl_cur = arr ? 2 * stb__sbm(arr) : 0;
 int min_needed = sb_count(arr) + increment;
 int m = dbl_cur > min_needed ? dbl_cur : min_needed;
 int *p = (int *) realloc(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int) * 2);
 if (p) {
    if (!arr)
       p[1] = 0;
    p[0] = m;
    return p + 2;
 } else {
    return (void *) (2  *sizeof(int));
 }
}
 
int main (int argc, char **argv) {
    ULL nr = 0; char *buf = NULL;
    int cnt = 0;
    FILE *fd = fopen (argv[1], "rb");
    do {
        char *str = calloc(BUFFER_SIZE, sizeof(char));
        nr = fread(str, sizeof(char), BUFFER_SIZE, fd);
	int n = feof (fd);
        printf(" nr:%d n:%d \n\r",nr,n);
	while (*str) sb_push(buf, *str++);
        //free(str - nr);
    } while (!feof (fd));
    printf("%s\n", buf); 
    sb_free(buf);
    return 0;
}
