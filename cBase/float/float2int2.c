#include <stdio.h>
#include <math.h>
int main() {
    double x=3.1415, intpart;  //为变量赋初值
    double fractpart = modf(x, &intpart);  //求3.1415的小数部分
    printf("intpart: %lf\nfractpart: %lf\n", intpart, fractpart);
    return 0;
}
