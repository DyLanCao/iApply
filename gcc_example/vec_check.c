#include <stdio.h>
#include <math.h>

#if defined(__clang__)
typedef float v4sf __attribute__((ext_vector_type(4)));
#else
typedef float v4sf __attribute__ ((vector_size (4)));
#endif

void print_v4sf(v4sf a) { for(int i=0; i<4; i++) printf("%f ", a[i]); puts(""); }

int main(void) {
  v4sf a;
  //broadcast a scalar
  a = ((v4sf){} + 1)*3.14159f;  
  print_v4sf(a);

  // vector + scalar
  a += 3.14159f;
  print_v4sf(a);

  // vector*scalar
  a *= 3.14159f;
  print_v4sf(a);

  //load from array
  float data[] = {1, 2, 3, 4};
  a = *(v4sf*)data;
  //a = __builtin_ia32_loadups(data);

  //store to array
  float store[4];
  *(v4sf*)store = a;
  for(int i=0; i<4; i++) printf("%f ", store[i]); puts("");
}
