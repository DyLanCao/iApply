
#include <stdio.h>

void test_func(void *test_val)
{   
        int tmp_val = 20;

        int *test = &test_val;
        printf("tst_val:0x%x test:%d",&test_val,test);
}


int main()
{
        
    static int val = NULL;
    
    int *p = &val;

    test_func(p);

    printf("val is:%d\n\t",val);

    return 0;
}
