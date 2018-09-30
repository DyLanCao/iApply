#include<stdio.h>           
#include <time.h>               /*要包含的头文件*/

int main(int argc, char *argv[])
{
    /* Init  */
    clock_t start, end;
    long long cnt = 0;

    printf("time calc test start \n");
    start = clock();           /*记录起始时间*/

    while(cnt++ < 800000000);
/*
    *
    *
    * 函数进行的一些列操作
    *
    * */

    /* Final Status */
    end = clock();           /*记录结束时间*/
    {
        double seconds  =(double)(end - start)/CLOCKS_PER_SEC;
        fprintf(stderr, "Use time is: %.8fs CLOCKS_PER_SEC:%d\n", seconds,CLOCKS_PER_SEC);
    }
    return 0;
}
