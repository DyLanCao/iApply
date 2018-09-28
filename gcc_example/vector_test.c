#include <stdio.h>
#include <math.h>

typedef unsigned char  v4u __attribute__((vector_size (4)));
typedef long int v4si __attribute__ ((vector_size (16)));
void print_v4si(v4si a) { for(int i=0; i<4; i++) printf("%d ", a[i]); puts(""); }
void print_v4u(v4u a) { for(int i=0; i<4; i++) printf("%d ", a[i]); puts(""); }

int main()
{

	v4si a, b, c;
	v4u a_tst,b_tst;
	long l;

	a = ((v4si){0,0,0,0} + 1);    /* a = b + {1,1,1,1}; */
	//a = 2 * b;    /* a = {2,2,2,2} * b; */
    a_tst = ((v4u){12,13,14,15} + 1);
	//a = l + a;    /* Error, cannot convert long to int. */
	//printf("%d",a[0]);
	//print_v4si(a);
	print_v4u(a_tst);
}
