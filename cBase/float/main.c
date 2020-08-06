#include <stdio.h>

int main()
{
    double testa = 0.34545,testb = 0.001,testf = 0.0;
    short testd = 1234;
    short *teste = malloc(10*sizeof(short));

    double testc = testd;
    teste[1]  = 4;
    // this code is right
    testa = testb - (double)teste[1];
    // it's wrong way
    testf = testb - teste[1];
    printf("one teste:%lf teste:%d \n\r",(double)teste[1],teste[1]);
    printf("two testf:%f \n\r",testf);
    printf("three teste:%f testa:%lf \n\r",testf,testa);
    
    free(teste);

    return 0;

}
